#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    double* ptr = sf_malloc(sizeof(double));
    
    double* ptr2 = sf_malloc(sizeof(double));
    size_t* ptr3 = sf_malloc(3 * PAGE_SZ);
    sf_free(ptr3);
    size_t* ptr4 = sf_malloc(2 * PAGE_SZ);
    void *x = sf_malloc(16384 - 48 - (sizeof(sf_header) + sizeof(sf_footer)));

    *ptr = 320320320e-320;
    *ptr2 = 3.141592;

    sf_show_heap();

    ptr = sf_realloc(ptr, sizeof(double) * 2);
    ptr4 = sf_realloc(ptr4, 2 * PAGE_SZ);

    // printf("%f\n", *ptr);
    // printf("%f\n", *ptr2);
    
    sf_free(ptr);
    sf_free(ptr2);
    sf_free(ptr4);
    sf_free(x);

    sf_show_heap();

    return EXIT_SUCCESS;
}
