/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"

double total_payload = 0;
double total_block = 0;
double max_aggr_payload = 0;

size_t make_mem_row(size_t payload, size_t block_size, size_t alloc, size_t prv_alloc)
{
    size_t mem_row = 0x0;
    mem_row = mem_row | payload;
    mem_row = mem_row << 28;
    mem_row = mem_row << 1;
    mem_row = mem_row | alloc;
    mem_row = mem_row << 1;
    mem_row = mem_row | prv_alloc;
    mem_row = mem_row << 2;
    mem_row = mem_row | block_size;

    return mem_row;
}

int get_free_h_idx(sf_block* block)
{
    const size_t block_size_calculater = (make_mem_row(0,0xfffffff, 0, 0) << 4);
    size_t header_block_size = block->header & block_size_calculater;

    if((sf_block* )((void *)block + header_block_size) == (sf_block *) (sf_mem_end() - 16))
    {
        return (NUM_FREE_LISTS - 1);
    }

    int fb0 = 0, fb1 = 32;

    for(int i = 0; i < NUM_FREE_LISTS - 2; i++)
    {
        int fb2 = fb0 + fb1;

        if(header_block_size <= fb2)
        {
            return i;
        }

        fb0 = fb1;
        fb1 = fb2;
    }

    return (NUM_FREE_LISTS - 2);
}

void *sf_malloc(size_t size) {
    // To be implemented.
    if(size <= 0)   return NULL;

    for(int i = 0; i < NUM_FREE_LISTS; i++)
    {
        if(sf_free_list_heads[i].body.links.prev == NULL)
        {
            sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
        }
        if(sf_free_list_heads[i].body.links.next == NULL)
        {
            sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        }
    }

    size_t heap_size = sf_mem_end() - sf_mem_start();
    const size_t block_size_calculater = (make_mem_row(0,0xfffffff, 0, 0) << 4);

    sf_block* wilderness = sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.next;
    size_t wilderness_size = wilderness->header & block_size_calculater;

    if(heap_size == 0)
    {
        if(sf_mem_grow() == NULL)
        {
            printf("HI\n");
            sf_errno = ENOMEM;
            return NULL;
        }

        heap_size = sf_mem_end() - sf_mem_start();

        sf_block* prologue = sf_mem_start();
        sf_block* epilogue = sf_mem_end() - 16;
        wilderness = sf_mem_start() + 32;

        prologue->header = make_mem_row(0, 32, 1, 0);
        epilogue->header = make_mem_row(0, 0, 1, 0);

        wilderness_size = heap_size - 48;

        wilderness->prev_footer = prologue->header;
        wilderness->header = make_mem_row(0, wilderness_size, 0, 1);
        epilogue->prev_footer = wilderness->header;

        wilderness->body.links.prev = &sf_free_list_heads[NUM_FREE_LISTS - 1];
        wilderness->body.links.next = &sf_free_list_heads[NUM_FREE_LISTS - 1];

        sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.prev = wilderness;
        sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.next = wilderness;

        // printf("prologue: %p\n", prologue);
        // printf("epilogue: %p\n", epilogue);

    }

    // printf("wilderness_size: %ld\n", wilderness_size);

    size_t payload = size;
    size_t block_size = size + 16;

    if(block_size < 32) block_size = 32;
    else if(block_size % 16 == 0)    block_size = size + 16;
    else                             block_size = 16 * (block_size / 16) + 16;

    // printf("block_size: %ld\n", block_size);

    for(int i = 0; i < NUM_FREE_LISTS - 1; i++)
    {
        if(sf_free_list_heads[i].body.links.prev == NULL && sf_free_list_heads[i].body.links.next == NULL)  continue;

        sf_block* freed_check = &sf_free_list_heads[i];

        while(freed_check->body.links.next != &sf_free_list_heads[i])
        {
            if((freed_check->body.links.next->header & 0b1000) == 0b1000)   continue;

            sf_block* alloc_f_block = freed_check->body.links.next;
            size_t free_size = (alloc_f_block->header & block_size_calculater);
            // printf("free_size: %ld\n", free_size);
            if(free_size == block_size)
            {
                int prv_alloc = (alloc_f_block->header & 0b100);
                prv_alloc = prv_alloc >> 2;
                alloc_f_block->header = make_mem_row(payload, block_size, 1, prv_alloc);

                sf_block* alloc_f_block_next = (sf_block *)((void *)alloc_f_block + block_size);
                alloc_f_block_next->header = alloc_f_block_next->header | 0x4;
                alloc_f_block_next->prev_footer = alloc_f_block->header;

                alloc_f_block->body.links.prev->body.links.next = alloc_f_block->body.links.next;
                alloc_f_block->body.links.next->body.links.prev = alloc_f_block->body.links.prev;

                alloc_f_block->body.links.prev = NULL;
                alloc_f_block->body.links.next = NULL;

                total_payload += payload;
                total_block += block_size;

                if(max_aggr_payload < total_payload)    max_aggr_payload = total_payload;

                return alloc_f_block->body.payload;
            }

            else if (free_size > block_size)
            {
                int prv_alloc = (alloc_f_block->header & 0b100);
                prv_alloc = prv_alloc >> 2;
                if((free_size - block_size) < 32)
                {
                    alloc_f_block->header = make_mem_row(payload, free_size, 1, prv_alloc);
                    
                    sf_block* alloc_f_block_next = (sf_block *)((void *)alloc_f_block + free_size);
                    alloc_f_block_next->header = alloc_f_block_next->header | 0x4;
                    alloc_f_block_next->prev_footer = alloc_f_block->header;

                    alloc_f_block->body.links.prev->body.links.next = alloc_f_block->body.links.next;
                    alloc_f_block->body.links.next->body.links.prev = alloc_f_block->body.links.prev;

                    alloc_f_block->body.links.prev = NULL;
                    alloc_f_block->body.links.next = NULL;

                    total_payload += payload;
                    total_block += free_size;
                    if(max_aggr_payload < total_payload)    max_aggr_payload = total_payload;
                }
                else
                {
                    alloc_f_block->header = make_mem_row(payload, block_size, 1, prv_alloc);
                    sf_block* left_free_block = (sf_block *) ((void *)alloc_f_block + block_size);
                    // left_free_block->header = make_mem_row(0, free_size - block_size, 0, 1);
                    left_free_block->prev_footer = alloc_f_block->header;

                    sf_block* left_free_block_next = (sf_block *) ((void *)alloc_f_block + free_size);

                    if((left_free_block_next->header & 0b1000) == 0)
                    {
                        size_t left_free_block_next_size = left_free_block_next->header & block_size_calculater;
                        size_t new_free_block_size = free_size - block_size + left_free_block_next_size;
                        left_free_block->header = make_mem_row(0, new_free_block_size, 0, 1);
                        sf_block* left_free_block_next_next = (sf_block *) ((void *)left_free_block + new_free_block_size);
                        left_free_block_next_next->prev_footer = left_free_block->header;

                        left_free_block_next->body.links.prev->body.links.next = left_free_block_next->body.links.next;
                        left_free_block_next->body.links.next->body.links.prev = left_free_block_next->body.links.prev;

                        alloc_f_block->body.links.prev->body.links.next = alloc_f_block->body.links.next;
                        alloc_f_block->body.links.next->body.links.prev = alloc_f_block->body.links.prev;

                        alloc_f_block->body.links.prev = NULL;
                        alloc_f_block->body.links.next = NULL;

                        int idx = get_free_h_idx(left_free_block);

                        left_free_block->body.links.prev = &sf_free_list_heads[idx];
                        left_free_block->body.links.next = sf_free_list_heads[idx].body.links.next;

                        sf_free_list_heads[idx].body.links.next->body.links.prev = left_free_block;
                        sf_free_list_heads[idx].body.links.next = left_free_block;
                    }
                    else
                    {
                        left_free_block->header = make_mem_row(0, free_size - block_size, 0, 1);
                        left_free_block_next->prev_footer = left_free_block->header;

                        alloc_f_block->body.links.prev->body.links.next = alloc_f_block->body.links.next;
                        alloc_f_block->body.links.next->body.links.prev = alloc_f_block->body.links.prev;

                        alloc_f_block->body.links.prev = NULL;
                        alloc_f_block->body.links.next = NULL;

                        int idx = get_free_h_idx(left_free_block);

                        left_free_block->body.links.prev = &sf_free_list_heads[idx];
                        left_free_block->body.links.next = sf_free_list_heads[idx].body.links.next;

                        sf_free_list_heads[idx].body.links.next->body.links.prev = left_free_block;
                        sf_free_list_heads[idx].body.links.next = left_free_block;
                    }

                    total_payload += payload;
                    total_block += block_size;
                    if(max_aggr_payload < total_payload)    max_aggr_payload = total_payload;

                }

                return alloc_f_block->body.payload;
            }

            freed_check = freed_check->body.links.next;
        }
    }

    while(block_size > wilderness_size)
    {
        if(sf_mem_grow() == NULL)
        {
            sf_errno = ENOMEM;
            return NULL;
        }
        // printf("block_size: %ld\n", block_size);
        sf_block* epilogue = (sf_block*) ((void *)sf_mem_end() - 16);
        epilogue->header = make_mem_row(0, 0, 1, 0);

        wilderness_size = wilderness_size + PAGE_SZ;
        // printf("wilderness_size: %ld\n", wilderness_size);
        wilderness->header = make_mem_row(0, wilderness_size, 0, 1);
        epilogue->prev_footer = wilderness->header;
        // printf("Wilderness: %p\n", wilderness);
        // sf_show_heap();
    }

    sf_block* alloc_block = wilderness;
    int prv_alloc = (wilderness->header & 0b100);
    prv_alloc = prv_alloc >> 2;

    // printf("prv_alloc: %d\n", prv_alloc);

    alloc_block->header = make_mem_row(payload, block_size, 1, prv_alloc);
    alloc_block->prev_footer = wilderness->prev_footer;
    // wilderness = (sf_block *) ((void *)alloc_block + block_size);

    total_payload += payload;
    total_block += block_size;
    if(max_aggr_payload < total_payload)    max_aggr_payload = total_payload;

    wilderness_size = wilderness_size - block_size;

    // wilderness->prev_footer = alloc_block->header;
    // wilderness->header = make_mem_row(0, wilderness_size, 0, 1);

    alloc_block->body.links.prev = NULL;
    alloc_block->body.links.next = NULL;

    sf_block* new_wilderness = (sf_block *) ((void *)alloc_block + block_size);

    // printf("wilderness size: %ld\n", wilderness_size);

    new_wilderness->header = make_mem_row(0, wilderness_size, 0, 1);
    // printf("new wilderness: %lx\n", wilderness->header);
    new_wilderness->prev_footer = alloc_block->header;

    sf_block* epilogue = (sf_block*) ((void *)sf_mem_end() - 16);

    if(wilderness_size == 0)
    {
        epilogue->header = make_mem_row(0, 0, 1, 1);
        epilogue->prev_footer = new_wilderness->header;

        sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.prev = &sf_free_list_heads[NUM_FREE_LISTS - 1];
        sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.next = &sf_free_list_heads[NUM_FREE_LISTS - 1];
        // sf_show_heap();
        return alloc_block->body.payload;
    }
    else                        epilogue->header = make_mem_row(0, 0, 1, 0);

    epilogue->prev_footer = new_wilderness->header;

    // printf("epilogue: %p\n", epilogue);

    new_wilderness->body.links.prev = &sf_free_list_heads[NUM_FREE_LISTS - 1];
    new_wilderness->body.links.next = &sf_free_list_heads[NUM_FREE_LISTS - 1];

    sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.prev = new_wilderness;
    sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.next = new_wilderness;

    // sf_show_heap();

    return alloc_block->body.payload;

}

void sf_free(void *pp) {
    // To be implemented.
    if(pp == NULL)  abort();

    size_t payload_calculater = (make_mem_row(0xffffffff, 0, 0, 0));
    size_t block_size_calculater = (make_mem_row(0,0xfffffff, 0, 0) << 4);

    sf_block* free_block = (sf_block *) (pp - 16);
    size_t free_block_payload = (free_block->header & (payload_calculater)) >> 32;
    size_t free_block_size = (free_block->header & (block_size_calculater));
    size_t free_block_prev_size = free_block->prev_footer & (block_size_calculater);
    sf_block* free_block_prev = (sf_block *)((void *) free_block - free_block_prev_size);
    sf_block* free_block_next = (sf_block *) ((void *)free_block + free_block_size);
    size_t free_block_next_size = (free_block_next->header & (block_size_calculater));

    // printf("\n");
    // printf("<free_block_prev>");
    // sf_show_block(free_block_prev);
    // printf("\n");
    // printf("<free_block>");
    // sf_show_block(free_block);
    // printf("\n");
    // sf_show_block(free_block_next);
    // printf("<free_block_next>");
    // printf("\n");
    // printf("block_size: %ld\n", free_block_size);

    // printf("Before pointer Check\n");

    if(((size_t)free_block) % 16 != 0
        || free_block_size < 32
        || free_block_size % 16 != 0
        || (free_block->header & 0b1000) == 0
        || (((free_block->prev_footer & 0b1000) != 0) && ((free_block->header & 0b100) == 0)))
    {
        abort();
    }

    // printf("After pointer Check\n");

    // printf("free_block_prev_header_alloc: %lx\n", (free_block_prev->header & 0b1000));
    // printf("free_block_next_header_alloc: %lx\n", (free_block_next->header & 0b1000));

    total_payload -= free_block_payload;
    total_block -= free_block_size;
    // if(max_aggr_payload < total_payload)    max_aggr_payload = total_payload;

    if(((free_block_prev->header & 0b1000) != 0) && ((free_block_next->header & 0b1000) != 0))
    {   // prev and next block are all allocated blocks
        int idx = get_free_h_idx(free_block);

        free_block->header = make_mem_row(0, free_block_size, 0, 1);
        free_block_next->header = free_block_next->header & 0xfffffffffffffffb;
        free_block_next->prev_footer = free_block->header;

        free_block->body.links.prev = &sf_free_list_heads[idx];
        free_block->body.links.next = sf_free_list_heads[idx].body.links.next;

        sf_free_list_heads[idx].body.links.next->body.links.prev = free_block;
        sf_free_list_heads[idx].body.links.next = free_block;

    }
    else if(((free_block_prev->header & 0b1000) != 0) && ((free_block_next->header & 0b1000) == 0))
    {   // prev block is allcoated and next block is free

        sf_block* free_block_next_next = (sf_block* )((void *) free_block_next + free_block_next_size);
        free_block_size = free_block_size + free_block_next_size;

        free_block->header = make_mem_row(0, free_block_size, 0, 1);

        free_block_next_next->prev_footer = free_block->header;

        free_block_next->body.links.prev->body.links.next = free_block_next->body.links.next;
        free_block_next->body.links.next->body.links.prev = free_block_next->body.links.prev;

        int idx = get_free_h_idx(free_block);

        free_block->body.links.prev = &sf_free_list_heads[idx];
        free_block->body.links.next = sf_free_list_heads[idx].body.links.next;

        sf_free_list_heads[idx].body.links.next->body.links.prev = free_block;
        sf_free_list_heads[idx].body.links.next = free_block;

        // printf("sf_free_list_heads[%d].body.links.prev: %p\n", idx, sf_free_list_heads[idx].body.links.prev);
        // printf("sf_free_list_heads[%d].body.links.next: %p\n", idx, sf_free_list_heads[idx].body.links.next);
        // printf("&sf_free_list_heads[%d]               : %p\n", idx, &sf_free_list_heads[idx]);
    }
    else if(((free_block_prev->header & 0b1000) == 0) && ((free_block_next->header & 0b1000) != 0))
    {   // prev block is free and next block is allocated
        free_block_size = free_block_size + free_block_prev_size;

        free_block_prev->header = make_mem_row(0, free_block_size, 0, 1);
        free_block_next->prev_footer = free_block_prev->header;

        free_block_prev->body.links.prev->body.links.next = free_block_prev->body.links.next;
        free_block_prev->body.links.next->body.links.prev = free_block_prev->body.links.prev;

        int idx = get_free_h_idx(free_block_prev);

        free_block_prev->body.links.prev = &sf_free_list_heads[idx];
        free_block_prev->body.links.next = sf_free_list_heads[idx].body.links.next;

        sf_free_list_heads[idx].body.links.next->body.links.prev = free_block_prev;
        sf_free_list_heads[idx].body.links.next = free_block_prev;

    }
    else
    {   // prev and next block are all free
        sf_block* free_block_next_next = (sf_block* )((void *) free_block_next + free_block_next_size);
        free_block_size = free_block_size + free_block_prev_size + free_block_next_size;

        free_block_prev->header = make_mem_row(0, free_block_size, 0, 1);
        free_block_next_next->prev_footer = free_block_prev->header;

        free_block_prev->body.links.prev->body.links.next = free_block_prev->body.links.next;
        free_block_prev->body.links.next->body.links.prev = free_block_prev->body.links.prev;
        free_block_next->body.links.prev->body.links.next = free_block_next->body.links.next;
        free_block_next->body.links.next->body.links.prev = free_block_next->body.links.prev;

        int idx = get_free_h_idx(free_block_prev);

        free_block_prev->body.links.prev = &sf_free_list_heads[idx];
        free_block_prev->body.links.next = sf_free_list_heads[idx].body.links.next;

        sf_free_list_heads[idx].body.links.next->body.links.prev = free_block_prev;
        sf_free_list_heads[idx].body.links.next = free_block_prev;
    }

    // sf_show_heap();

}

void *sf_realloc(void *pp, size_t rsize) {
    // To be implemented.
    if(pp == NULL)
    {
        sf_errno = EINVAL;
        return NULL;
    }

    size_t payload_calculater = (make_mem_row(0xffffffff, 0, 0, 0));
    size_t block_size_calculater = (make_mem_row(0,0xfffffff, 0, 0) << 4);

    sf_block* alloc_block = (sf_block *) (pp - 16);
    size_t alloc_block_payload = (alloc_block->header & (payload_calculater)) >> 32;
    size_t alloc_block_size = (alloc_block->header & (block_size_calculater));
    // size_t alloc_block_prev_size = alloc_block->prev_footer & (block_size_calculater);
    // sf_block* alloc_block_prev = (sf_block *)((void *)alloc_block - alloc_block_prev_size);
    sf_block* alloc_block_next = (sf_block *) ((void *)alloc_block + alloc_block_size);
    // size_t alloc_block_next_size = (alloc_block_next->header & (block_size_calculater));



    if(((size_t)alloc_block) % 16 != 0
        || alloc_block_size < 32
        || alloc_block_size % 16 != 0
        || (alloc_block->header & 0b1000) == 0
        || (((alloc_block->prev_footer & 0b1000) != 0) && ((alloc_block->header & 0b100) == 0)))
    {
        sf_errno = EINVAL;
        return NULL;
    }
    // printf("alloc_block_payload: %ld\n", alloc_block_payload);
    // printf("rsize: %ld\n", rsize);

    if(rsize == 0)
    {
        sf_free(alloc_block);
        sf_errno = EINVAL;
        return NULL;
    }

    size_t realloc_block_size = rsize + 16;
    if(realloc_block_size < 32) realloc_block_size = 32;
    else if(realloc_block_size % 16 == 0)    realloc_block_size = rsize + 16;
    else                                     realloc_block_size = 16 * (realloc_block_size / 16) + 16;

    if(realloc_block_size > alloc_block_size)
    {
        sf_block* realloc_block = (sf_block *)(sf_malloc(rsize) - 16);
        if(realloc_block == NULL)   return NULL;
        memcpy(realloc_block->body.payload, pp, alloc_block_payload);
        sf_free(pp);

        return realloc_block->body.payload;
    }
    else
    {
        if(alloc_block_size - realloc_block_size >= 32)
        {
            size_t free_block_size = alloc_block_size - realloc_block_size;
            int prv_alloc = (alloc_block->header & 0b100);
            prv_alloc = prv_alloc >> 2;
            alloc_block->header = make_mem_row(rsize, realloc_block_size, 1, prv_alloc);

            sf_block* left_free_block = (sf_block *)((void *)alloc_block + realloc_block_size);
            left_free_block->header = make_mem_row(0, free_block_size, 0, 1);
            left_free_block->prev_footer = alloc_block->header;
            alloc_block_next->prev_footer = left_free_block->header;

            total_payload += rsize;
            total_block += realloc_block_size;
            total_payload -= alloc_block_payload;
            total_block -= alloc_block_size;
            if(max_aggr_payload < total_payload)    max_aggr_payload = total_payload;

            if((alloc_block_next->header & 0b1000) == 0)
            {
                size_t alloc_block_next_size = alloc_block_next->header & block_size_calculater;
                size_t new_free_block_size = free_block_size + alloc_block_next_size;
                left_free_block->header = make_mem_row(0, new_free_block_size, 0, 1);
                sf_block* left_free_block_next_next = (sf_block *) ((void *)left_free_block + new_free_block_size);

                left_free_block_next_next->prev_footer = left_free_block->header;

                alloc_block_next->body.links.prev->body.links.next = alloc_block_next->body.links.next;
                alloc_block_next->body.links.next->body.links.prev = alloc_block_next->body.links.prev;

                int idx = get_free_h_idx(left_free_block);

                left_free_block->body.links.prev = &sf_free_list_heads[idx];
                left_free_block->body.links.next = sf_free_list_heads[idx].body.links.next;

                sf_free_list_heads[idx].body.links.next->body.links.prev = left_free_block;
                sf_free_list_heads[idx].body.links.next = left_free_block;
            }
            else
            {
                left_free_block->header = make_mem_row(0, free_block_size, 0, 1);

                int idx = get_free_h_idx(left_free_block);

                left_free_block->body.links.prev = &sf_free_list_heads[idx];
                left_free_block->body.links.next = sf_free_list_heads[idx].body.links.next;

                sf_free_list_heads[idx].body.links.next->body.links.prev = left_free_block;
                sf_free_list_heads[idx].body.links.next = left_free_block;
            }

            // sf_show_heap();
            return alloc_block->body.payload;
        }
        else
        {
            int prv_alloc = (alloc_block->header & 0b100);
            prv_alloc = prv_alloc >> 2;
            alloc_block->header = make_mem_row(rsize, alloc_block_size, 1, prv_alloc);
            alloc_block_next->prev_footer = alloc_block->header;

            
            total_payload += rsize;
            total_payload -= alloc_block_payload;
            if(max_aggr_payload < total_payload)    max_aggr_payload = total_payload;
            
            // sf_show_heap();
            return alloc_block->body.payload;
        }
    }

    return NULL;
}

double sf_fragmentation() {
    // To be implemented.

    // printf("total payload: %f\n", total_payload);
    // printf("total block:        %f\n", total_block);

    if(total_block == 0)    return 0.0;
    else return (total_payload / total_block);
}

double sf_utilization() {
    // To be implemented.

    size_t heap_size = sf_mem_end() - sf_mem_start();

    // printf("max_aggr_payload: %f\n", max_aggr_payload);
    // printf("heap size:        %ld\n", heap_size);

    if(heap_size == 0)  return 0.0;
    else return (double)(max_aggr_payload / heap_size);
}
