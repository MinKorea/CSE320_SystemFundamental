#include <stdlib.h>

#include "global.h"
#include "debug.h"



double ten_pow(int x)
{
    double result = 1;

    if(x >= 0)
    {
        for(int i = 0; i < x; i++)  result *= 10;
    }
    else
    {
        x *= -1;
        for(int i = 0; i < x; i++)  result /= 10;
    }

    return result;
}

double str_to_double(char *c)
{
    double result = 0;
    int digit = -1;
    int length = 0;
    int idx = 0;
    int negative_flag = 0;
    int fraction_flag = 0;

    while(*(c + idx) != 0)
    {
        length++;

        if(*(c + idx) == '.' || *(c + idx) == '-')
        {
            if(*(c + idx) == '.')   fraction_flag = 1;
            idx++;
            continue;
        }

        if(fraction_flag == 0) digit++;
        idx++;
    }

    if(*c == '-')   negative_flag = 1;

    for(int i = 0; i < length; i++)
    {
        if(*c == '.' || *c == '-')
        {
            c++;
            i--;
            length--;
            continue;
        }
        int digit_cal = digit - i;

        result += ((double)(*c - '0') * ten_pow(digit_cal));
        c++;
    }

    if(negative_flag != 0)  result *= -1;

    return result;

}

/**
 * @brief  Read genetic distance data and initialize data structures.
 * @details  This function reads genetic distance data from a specified
 * input stream, parses and validates it, and initializes internal data
 * structures.
 *
 * The input format is a simplified version of Comma Separated Values
 * (CSV).  Each line consists of text characters, terminated by a newline.
 * Lines that start with '#' are considered comments and are ignored.
 * Each non-comment line consists of a nonempty sequence of data fields;
 * each field iis terminated either by ',' or else newline for the last
 * field on a line.  The constant INPUT_MAX specifies the maximum number
 * of data characters that may be in an input field; fields with more than
 * that many characters are regarded as invalid input and cause an error
 * return.  The first field of the first data line is empty;
 * the subsequent fields on that line specify names of "taxa", which comprise
 * the leaf nodes of a phylogenetic tree.  The total number N of taxa is
 * equal to the number of fields on the first data line, minus one (for the
 * blank first field).  Following the first data line are N additional lines.
 * Each of these lines has N+1 fields.  The first field is a taxon name,
 * which must match the name in the corresponding column of the first line.
 * The subsequent fields are numeric fields that specify N "distances"
 * between this taxon and the others.  Any additional lines of input following
 * the last data line are ignored.  The distance data must form a symmetric
 * matrix (i.e. D[i][j] == D[j][i]) with zeroes on the main diagonal
 * (i.e. D[i][i] == 0).
 *
 * If 0 is returned, indicating data successfully read, then upon return
 * the following global variables and data structures have been set:
 *   num_taxa - set to the number N of taxa, determined from the first data line
 *   num_all_nodes - initialized to be equal to num_taxa
 *   num_active_nodes - initialized to be equal to num_taxa
 *   node_names - the first N entries contain the N taxa names, as C strings
 *   distances - initialized to an NxN matrix of distance values, where each
 *     row of the matrix contains the distance data from one of the data lines
 *   nodes - the "name" fields of the first N entries have been initialized
 *     with pointers to the corresponding taxa names stored in the node_names
 *     array.
 *   active_node_map - initialized to the identity mapping on [0..N);
 *     that is, active_node_map[i] == i for 0 <= i < N.
 *
 * @param in  The input stream from which to read the data.
 * @return 0 in case the data was successfully read, otherwise -1
 * if there was any error.  Premature termination of the input data,
 * failure of each line to have the same number of fields, and distance
 * fields that are not in numeric format should cause a one-line error
 * message to be printed to stderr and -1 to be returned.
 */



int read_distance_data(FILE *in) {
    // TO BE IMPLEMENTED
    if(in != NULL)
    {
        int col_cnt = 0;
        int row_cnt = 0;
        int chr_cnt = 0;
        num_taxa = 0;
        char c;

        while(!feof(in))
        {
            c = fgetc(in);

            if(c == '#')
            {
                while(c != '\n')    c = fgetc(in);
                continue;
            }
            else
            {
                int i = 0;

                while(col_cnt == 0)
                {
                    if(c == ',')
                    {
                        c = fgetc(in);

                        if(i != 0)
                        {
                            num_taxa++;
                        }
                        i = 0;
                        continue;
                    }
                    else
                    {
                        *(*(node_names + num_taxa) + i) = c;
                        c = fgetc(in);
                        i++;
                    }

                    if(c == '\n')
                    {
                        num_taxa++;
                        col_cnt++;
                    }
                }

                num_all_nodes = num_taxa;
                num_active_nodes = num_taxa;

                for(int i = 0; i < num_taxa; i++)
                {
                    if(feof(in))    break;

                    row_cnt = 0;

                    while(row_cnt == 0)
                    {
                        c = fgetc(in);
                        if(c == ',')    row_cnt++;
                    }

                     c = fgetc(in);

                    for(int j = 0; j < num_taxa; j++)
                    {
                        if(c == ',')    c = fgetc(in);
                        else if(c == '\n')
                        {
                            c = fgetc(in);
                            break;
                        }

                        int num_len = 0;
                        double distance;

                        while(c != ',' && c != '\n')
                        {
                            *(input_buffer + num_len) = c;
                            num_len++;
                            c = fgetc(in);
                        }
                        *(input_buffer + num_len) = 0;
                        distance = str_to_double(input_buffer);
                        *(*(distances + i) + j) = distance;
                    }
                }
            }
        }

        for(int i = 0; i < num_taxa; i++)
        {
            *(active_node_map + i) = i;

            (nodes + i)->name = *(node_names + i);
        }

        for(int i = 0; i < num_taxa; i++)
        {
            if(*(*(distances + i) + i) != 0)    return -1;
            for(int j = 0; j <= i; j++)
            {
                if(*(*(distances + i) + j) != *(*(distances + j) + i))
                    return -1;
            }
        }

        return 0;
    }
    return -1;
}

/**
 * @brief  Emit a representation of the phylogenetic tree in Newick
 * format to a specified output stream.
 * @details  This function emits a representation in Newick format
 * of a synthesized phylogenetic tree to a specified output stream.
 * See (https://en.wikipedia.org/wiki/Newick_format) for a description
 * of Newick format.  The tree that is output will include for each
 * node the name of that node and the edge distance from that node
 * its parent.  Note that Newick format basically is only applicable
 * to rooted trees, whereas the trees constructed by the neighbor
 * joining method are unrooted.  In order to turn an unrooted tree
 * into a rooted one, a root will be identified according by the
 * following method: one of the original leaf nodes will be designated
 * as the "outlier" and the unique node adjacent to the outlier
 * will serve as the root of the tree.  Then for any other two nodes
 * adjacent in the tree, the node closer to the root will be regarded
 * as the "parent" and the node farther from the root as a "child".
 * The outlier node itself will not be included as part of the rooted
 * tree that is output.  The node to be used as the outlier will be
 * determined as follows:  If the global variable "outlier_name" is
 * non-NULL, then the leaf node having that name will be used as
 * the outlier.  If the value of "outlier_name" is NULL, then the
 * leaf node having the greatest total distance to the other leaves
 * will be used as the outlier.
 *
 * @param out  Stream to which to output a rooted tree represented in
 * Newick format.
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.  If the global variable "outlier_name" is
 * non-NULL, then it is an error if no leaf node with that name exists
 * in the tree.
 */

int get_cur_idx(char *name, int num_of_names)
{
    char *c = name;

    for(int i = 0; i < num_of_names; i++)
    {
        int char_cnt = 0;

        char *c2 = *(node_names + i);

        while(*c != 0 && *c2 != 0)
        {
            if(*(c + char_cnt) != *c2)                 break;
            if(*(c + char_cnt) == 0 && *c2 != 0)       break;
            else if(*(c + char_cnt) != 0 && *c2 == 0)  break;

            if(*((c + char_cnt) + 1) == 0 && *(c2 + 1) == 0)
            {
                return i;
            }

            char_cnt++;
            c2++;
        }
    }



    return -1;  // if nothing matches
}

void newick_print_sub(FILE *out, NODE *cur_node, NODE *prt_node)
{
    int cur_idx = get_cur_idx(cur_node->name, num_all_nodes);
    int prt_idx = get_cur_idx(prt_node->name, num_all_nodes);

    for(int i = 0; i < 3; i++)
    {

        if(*((cur_node)->neighbors + i) == prt_node)
        {
            continue;
        }

        if(*((cur_node)->neighbors + i) != NULL
            && *((cur_node)->neighbors + i) != prt_node)
        {
            if(i == 0)  fprintf(out, "(");
            else if(i == 1
                && *((cur_node)->neighbors + i - 1) == prt_node)
                        fprintf(out, "(");
            else        fprintf(out, ",");

            newick_print_sub(out, *(cur_node->neighbors + i), cur_node);

            if(i == 2)  fprintf(out, ")");
            else if(*((prt_node)->neighbors + i) == NULL)
                fprintf(out, ")");

            }
    }

    fprintf(out, "%s:", *(node_names + cur_idx));
    fprintf(out, "%.2f", *(*(distances + cur_idx) + prt_idx));
}

int emit_newick_format(FILE *out) {
    // TO BE IMPLEMENTED

    int default_node_idx = 0;

    double greatest = 0;

    for(int i = 0; i < num_taxa; i++)
    {
        for(int j = 0; j < num_taxa; j++)
        {
            double temp = *(*(distances + i) + j);
            if(temp > greatest)
            {
                greatest = temp;
                default_node_idx = (i < j) ? i : j;
            }
        }
    }

    // printf("Default Node: %s\n", *(node_names + default_node_idx));

    if(outlier_name != 0)
    {
        default_node_idx = get_cur_idx(outlier_name, num_taxa);
        if(default_node_idx == -1)  return -1;
    }

    // printf("Default Node: %s\n", (*(nodes + default_node_idx)).name);
    newick_print_sub(out, *((nodes + default_node_idx)->neighbors + 0), (nodes + default_node_idx));
    fprintf(out, "\n");
    return 0;
}

/**
 * @brief  Emit the synthesized distance matrix as CSV.
 * @details  This function emits to a specified output stream a representation
 * of the synthesized distance matrix resulting from the neighbor joining
 * algorithm.  The output is in the same CSV form as the program input.
 * The number of rows and columns of the matrix is equal to the value
 * of num_all_nodes at the end of execution of the algorithm.
 * The submatrix that consists of the first num_leaves rows and columns
 * is identical to the matrix given as input.  The remaining rows and columns
 * contain estimated distances to internal nodes that were synthesized during
 * the execution of the algorithm.
 *
 * @param out  Stream to which to output a CSV representation of the
 * synthesized distance matrix.
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.
 */
int emit_distance_matrix(FILE *out) {
    // TO BE IMPLEMENTED
    for(int i = 0; i < num_all_nodes; i++)
    {
        fprintf(out, ",%s", *(node_names + i));
    }

    fprintf(out, "\n");

    for(int i = 0; i < num_all_nodes; i++)
    {
        fprintf(out, "%s,", *(node_names + i));
        for(int j = 0; j < num_all_nodes - 1; j++)
        {
            fprintf(out, "%.2f,", *(*(distances + i) + j));
        }
        fprintf(out, "%.2f", *(*(distances + i) + num_all_nodes - 1));
        printf("\n");
    }

    return 0;
}

/**
 * @brief  Build a phylogenetic tree using the distance data read by
 * a prior successful invocation of read_distance_data().
 * @details  This function assumes that global variables and data
 * structures have been initialized by a prior successful call to
 * read_distance_data(), in accordance with the specification for
 * that function.  The "neighbor joining" method is used to reconstruct
 * phylogenetic tree from the distance data.  The resulting tree is
 * an unrooted binary tree having the N taxa from the original input
 * as its leaf nodes, and if (N > 2) having in addition N-2 synthesized
 * internal nodes, each of which is adjacent to exactly three other
 * nodes (leaf or internal) in the tree.  As each internal node is
 * synthesized, information about the edges connecting it to other
 * nodes is output.  Each line of output describes one edge and
 * consists of three comma-separated fields.  The first two fields
 * give the names of the nodes that are connected by the edge.
 * The third field gives the distance that has been estimated for
 * this edge by the neighbor-joining method.  After N-2 internal
 * nodes have been synthesized and 2*(N-2) corresponding edges have
 * been output, one final edge is output that connects the two
 * internal nodes that still have only two neighbors at the end of
 * the algorithm.  In the degenerate case of N=1 leaf, the tree
 * consists of a single leaf node and no edges are output.  In the
 * case of N=2 leaves, then no internal nodes are synthesized and
 * just one edge is output that connects the two leaves.
 *
 * Besides emitting edge data (unless it has been suppressed),
 * as the tree is built a representation of it is constructed using
 * the NODE structures in the nodes array.  By the time this function
 * returns, the "neighbors" array for each node will have been
 * initialized with pointers to the NODE structure(s) for each of
 * its adjacent nodes.  Entries with indices less than N correspond
 * to leaf nodes and for these only the neighbors[0] entry will be
 * non-NULL.  Entries with indices greater than or equal to N
 * correspond to internal nodes and each of these will have non-NULL
 * pointers in all three entries of its neighbors array.
 * In addition, the "name" field each NODE structure will contain a
 * pointer to the name of that node (which is stored in the corresponding
 * entry of the node_names array).
 *
 * @param out  If non-NULL, an output stream to which to emit the edge data.
 * If NULL, then no edge data is output.
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.
 */
int build_taxonomy(FILE *out) {
    // TO BE IMPLEMENTED
    while(num_active_nodes > 2)
    {
        for(int i = 0; i < num_active_nodes; i++)      // Getting row_sums
        {
            double sum = 0;

            for(int j = 0; j < num_active_nodes; j++)
            {
                sum += *(*(distances + *(active_node_map + i)) + *(active_node_map + j));
            }

            *(row_sums + *(active_node_map + i)) = sum;

            // printf("row_sums[%s]: %.2f\n", *(node_names + *(active_node_map + i)), *(row_sums + *(active_node_map + i)));
        }

        double q_low = 1;

        int q_idx_i = 0, q_idx_j = 0;
        int act_i = 0, act_j = 0;

        for (int i = 0; i < num_active_nodes; i++)
        {
            for (int j = 0; j < num_active_nodes; j++)
            {
                if(i == j)
                {
                    // printf("0.00\t");
                    continue;
                }

                double temp = (num_active_nodes - 2) * (*(*(distances + *(active_node_map + i)) + *(active_node_map + j)))
                - (*(row_sums + *(active_node_map + i))) - (*(row_sums + *(active_node_map + j)));

                // printf("D(%d, %d): %.2f\t", *(active_node_map + i), *(active_node_map + j), *(*(distances + *(active_node_map + i)) + *(active_node_map + j)));
                // printf("S(%d): %.2f\t", *(active_node_map + i), *(row_sums + *(active_node_map + i)));
                // printf("S(%d): %.2f\t", *(active_node_map + j), *(row_sums + *(active_node_map + j)));
                // printf("%.2f\t", temp);

                if(temp < q_low)
                {
                    q_low = temp;
                    q_idx_i = *(active_node_map + i);
                    act_i = i;
                    q_idx_j = *(active_node_map + j);
                    act_j = j;
                }
            }
            // printf("\n");
        }


        *(*(node_names + num_all_nodes)) = '#';

        int conv_num = num_all_nodes;
        int conv_num_idx = 1;

        while(conv_num > 0)
        {
            int temp_conv_num = conv_num;
            int conv_num_digit = 1;

            while(temp_conv_num >= 10)
            {
                temp_conv_num /= 10;
                conv_num_digit *= 10;
            }

            int div = conv_num / conv_num_digit;
            *(*(node_names + num_all_nodes) + conv_num_idx) = div + '0';

            conv_num %= conv_num_digit;
            conv_num_idx++;

            if(conv_num_digit > 1 && conv_num == 0)
            {
                while(conv_num_digit > 1)
                {
                    *(*(node_names + num_all_nodes) + conv_num_idx) = '0';
                    conv_num_digit /= 10;
                    conv_num_idx++;
                }
                break;
            }

        }

        (*(nodes + num_all_nodes)).name = *(node_names + num_all_nodes);
        // Setting new node's children
        *((*(nodes + num_all_nodes)).neighbors + 1) = nodes + q_idx_i;
        *((*(nodes + num_all_nodes)).neighbors + 2) = nodes + q_idx_j;
        // Setting nodes' parent
        *((*(nodes + q_idx_i)).neighbors + 0) = nodes + num_all_nodes;
        *((*(nodes + q_idx_j)).neighbors + 0) = nodes + num_all_nodes;

        *(active_node_map + num_all_nodes) = num_all_nodes;

        for(int i = 0; i <= num_active_nodes; i++)
        {
            if(*(active_node_map + i) == num_all_nodes)
                *(*(distances + num_all_nodes) + *(active_node_map + i)) = 0;
            else if(*(active_node_map + i) == q_idx_i)
                {
                    *(*(distances + num_all_nodes) + *(active_node_map + i))
                                    = (*(*(distances + q_idx_i) + q_idx_j) + (*(row_sums + q_idx_i) - *(row_sums + q_idx_j)) / (num_active_nodes - 2)) / 2;

                    if(global_options == 0x0)
                        fprintf(out, "%d,%d,%.2f\n", *(active_node_map + i), num_all_nodes, *(*(distances + num_all_nodes) + *(active_node_map + i)));
                }
            else if(*(active_node_map + i) == q_idx_j)
                {
                    *(*(distances + num_all_nodes) + *(active_node_map + i))
                                    = (*(*(distances + q_idx_i) + q_idx_j) + (*(row_sums + q_idx_j) - *(row_sums + q_idx_i)) / (num_active_nodes - 2)) / 2;

                    if(global_options == 0x0)
                        fprintf(out, "%d,%d,%.2f\n", *(active_node_map + i), num_all_nodes, *(*(distances + num_all_nodes) + *(active_node_map + i)));
                }
            else
                {
                    *(*(distances + num_all_nodes) + *(active_node_map + i))
                                    = (*(*(distances + q_idx_i) + *(active_node_map + i)) + *(*(distances + q_idx_j) + *(active_node_map + i)) - *(*(distances + q_idx_i) + q_idx_j)) / 2;
                }

            *(*(distances + *(active_node_map + i)) + num_all_nodes) = *(*(distances + num_all_nodes) + *(active_node_map + i));

        }

        /*printf("Before Merge: \n");
        for(int i = 0; i < num_all_nodes; i++)
        {
            printf("Active node[%d]: %d\t", i, *(active_node_map + i));
        }
        printf("\n");*/


        /**(active_node_map + act_i) = q_idx_i;
        *(active_node_map + act_j) = q_idx_j;*/
        *(active_node_map + act_i) = num_all_nodes;
        *(active_node_map + act_j) = *(active_node_map + num_active_nodes - 1);

        // printf("After Merge: \n");

        num_all_nodes++;
        num_active_nodes--;

        /*for(int i = 0; i < num_all_nodes; i++)
        {
            printf("Active node[%d]: %d\t", i, *(active_node_map + i));
        }
        printf("\n");*/
    }

    *((*(nodes + num_all_nodes - 1)).neighbors + 0) = nodes + *(active_node_map + 1);

    if(global_options == 0x0)
        fprintf(out, "%d,%d,%.2f\n", *(active_node_map + 1), *(active_node_map), *(*(distances + *(active_node_map + 1)) + *(active_node_map)));


    /*for(int i = 0; i < num_all_nodes; i++)
        {
            printf("%s\t", *(node_names + i));
            for(int j = 0; j < num_all_nodes; j++)
            {
                printf("%.2f\t", distances[i][j]);
            }
            printf("\n");
        }*/

    return 0;
}
