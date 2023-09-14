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
        read_distance_data(stdin);

        build_taxonomy(stdout);

        if(global_options == MATRIX_OPTION)
        {
            emit_distance_matrix(stdout);
        }
        else if(global_options == NEWICK_OPTION)
        {
            emit_newick_format(stdout);
        }
    }

    return EXIT_FAILURE;

}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
