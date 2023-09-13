#include <stdlib.h>

#include "global.h"
#include "debug.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
int validargs(int argc, char **argv)
{
    if(*(*argv) == 'b' && *((*argv) + 1) == 'i' && *((*argv) + 2) == 'n' && *((*argv) + 3) == '/'
        && *((*argv) + 4) == 'p' && *((*argv) + 5) == 'h' && *((*argv) + 6) == 'i' && *((*argv) + 7) == 'l' && *((*argv) + 8) == 'o')
    {
        if(*(argv + 1) == 0)
        {
            return 0;
        }

        if(*(*(argv + 1)) == '-')
        {
            if(*(*(argv + 1) + 1) == 'h')
            {
                global_options = HELP_OPTION;
                return 0;
            }
            else
            {
                if(*(*(argv + 1) + 1) == 'm')
                {
                    if(*(argv + 2) != 0)
                    {
                        global_options = 0x0;
                        return -1;
                    }

                    global_options = MATRIX_OPTION;
                    return 0;
                }
                else if(*(*(argv + 1) + 1)== 'n')
                {
                    if(*(*(argv + 2)) == '-' && *(*(argv + 2) + 1) == 'o')
                    {
                        if(*(argv+ 3) == 0)
                        {
                            global_options = 0x0;
                            return -1;
                        }
                        else    outlier_name = *(argv + 3);

                        global_options = NEWICK_OPTION;
                        return 0;
                    }
                    else if(*(argv+ 3) != 0)
                    {
                        global_options = 0x0;
                        return -1;
                    }
                    global_options = NEWICK_OPTION;
                    return 0;

                }
                global_options = 0x0;
                return -1;
            }
        }
        else
        {
            global_options = 0x0;
            return -1;
        }

    }
    else
    {
        global_options = 0x0;
        return -1;
    }

}
