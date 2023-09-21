#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "debug.h"

int main(int argc, char **argv)
{
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if(global_options == HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    else
    {
        if(read_distance_data(stdin) == 0)
        {
            if(build_taxonomy(stdout) == 0)
            {
                if(global_options == MATRIX_OPTION)
                {
                    if(emit_distance_matrix(stdout) == 0)
                        return EXIT_SUCCESS;
                    else
                        return EXIT_FAILURE;
                }
                else if(global_options == NEWICK_OPTION)
                {
                    if(emit_newick_format(stdout) == 0)
                        return EXIT_SUCCESS;
                    else
                        return EXIT_FAILURE;
                }

                return EXIT_SUCCESS;
            }
            else return EXIT_FAILURE;
        }
        else return EXIT_FAILURE;
    }

    return EXIT_FAILURE;

}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
