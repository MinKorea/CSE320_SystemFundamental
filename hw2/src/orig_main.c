
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "version.h"
#include "global.h"
#include "gradedb.h"
#include "stats.h"
#include "read.h"
#include "write.h"
#include "normal.h"
#include "sort.h"
#include "error.h"
#include "report.h"

/*
 * Course grade computation program
 */

#define REPORT          0
#define COLLATE         1
#define FREQUENCIES     2
#define QUANTILES       3
#define SUMMARIES       4
#define MOMENTS         5
#define COMPOSITES      6
#define INDIVIDUALS     7
#define HISTOGRAMS      8
#define TABSEP          9
#define ALLOUTPUT      10
#define SORTBY         11
#define NONAMES        12
#define OUTPUT         13

static struct option_info {
        unsigned int val;
        char *name;
        char chr;
        int has_arg;
        char *argname;
        char *descr;
} option_table[] = {
 {REPORT,         "report",    'r',      no_argument, NULL,
                  "Process input data and produce specified reports."},
 {COLLATE,        "collate",   'c',      no_argument, NULL,
                  "Collate input data and dump to standard output."},
 {FREQUENCIES,    "freqs",     0,        no_argument, NULL,
                  "Print frequency tables."},
 {QUANTILES,      "quants",    0,        no_argument, NULL,
                  "Print quantile information."},
 {SUMMARIES,      "summaries", 0,        no_argument, NULL,
                  "Print quantile summaries."},
 {MOMENTS,        "stats",     0,        no_argument, NULL,
                  "Print means and standard deviations."},
 {COMPOSITES,     "comps",     0,        no_argument, NULL,
                  "Print students' composite scores."},
 {INDIVIDUALS,    "indivs",    0,        no_argument, NULL,
                  "Print students' individual scores."},
 {HISTOGRAMS,     "histos",    0,        no_argument, NULL,
                  "Print histograms of assignment scores."},
 {TABSEP,         "tabsep",    0,        no_argument, NULL,
                  "Print tab-separated table of student scores."},
 {ALLOUTPUT,      "all",       'a',      no_argument, NULL,
                  "Print all reports."},
 {SORTBY,         "sortby",    'k',      required_argument, "key",
                  "Sort by {name, id, score}."},
 {NONAMES,        "nonames",   'n',      no_argument, NULL,
                  "Suppress printing of students' names."},
 {OUTPUT,         "output",    'o',      required_argument, "file",
                  "Specify file to be used for output."}
};

static char short_options[14] = "";
static struct option long_options[15];

char colon = ':';

static void init_options() {
    for(unsigned int i = 0; i < 15; i++) {
        struct option *op = &long_options[i];
        if(i == 14)
        {
            op->name = NULL;
            op->has_arg = 0;
            op->flag = NULL;
            op->val = 0;
            break;
        }

        struct option_info *oip = &option_table[i];
        if(oip->val != i) {
            fprintf(stderr, "Option initialization error\n");
            abort();
        }
        op->name = oip->name;
        op->has_arg = oip->has_arg;
        op->flag = NULL;
        op->val = oip->val;
        char* c = &oip->chr;
        if(*c == 'o' || *c == 'k')
        {
            char* add_colon = &colon;
            strcat(c, add_colon);
            strcat(short_options, c);
        }
        else
        {
            strcat(short_options, c);
        }
    }

    // printf("short_options: %s\n", short_options);
}

static int report, collate, freqs, quantiles, summaries, moments,
           scores, composite, histograms, tabsep, nonames;

static void usage();

// int errors, warnings;

int orig_main(argc, argv)
int argc;
char *argv[];
{
    Course *c;
    Stats *s;
    char optval;
    int (*compare)() = comparename;

    fprintf(stderr, BANNER);
    init_options();
    if(argc <= 1) usage(argv[0]);
    while(optind < argc) {
        if((optval = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
            switch(optval) {
            case REPORT:
            case 'r':
                report++; break;
            case COLLATE:
            case 'c':
                collate++; break;
            case TABSEP: tabsep++; break;
            case NONAMES:
            case 'n':
                nonames++; break;
            case SORTBY:
            case 'k':
                printf("optarg: %s\n", optarg);
                if(!strcmp(optarg, "name"))
                    compare = comparename;
                else if(!strcmp(optarg, "id"))
                    compare = compareid;
                else if(!strcmp(optarg, "score"))
                    compare = comparescore;
                else {
                    fprintf(stderr,
                            "Option '%s' requires argument from {name, id, score}.\n\n",
                            option_table[SORTBY].name);
                    usage(argv[0]);
                }
                break;
            case FREQUENCIES: freqs++; break;
            case QUANTILES: quantiles++; break;
            case SUMMARIES: summaries++; break;
            case MOMENTS: moments++; break;
            case COMPOSITES: composite++; break;
            case INDIVIDUALS: scores++; break;
            case HISTOGRAMS: histograms++; break;
            case ALLOUTPUT:
            case OUTPUT:
            case 'o':
                stdout = fopen(optarg, "w");
                break;
            case 'a':
                freqs++; quantiles++; summaries++; moments++;
                composite++; scores++; histograms++; tabsep++;
                break;
            case '?':
                usage(argv[0]);
                break;
            default:
                break;
            }
        } else {
            break;
        }
    }
    if(optind == argc) {
            fprintf(stderr, "No input file specified.\n\n");
            usage(argv[0]);
    }
    char *ifile = argv[optind];
    if(report == collate) {
            fprintf(stderr, "Exactly one of '%s' or '%s' is required.\n\n",
                    option_table[REPORT].name, option_table[COLLATE].name);
            usage(argv[0]);
    }

    // free(short_options);

    fprintf(stderr, "Reading input data...\n");
    c = readfile(ifile);

    if(errors) {
       printf("%d error%s found, so no computations were performed.\n",
              errors, errors == 1 ? " was": "s were");
       exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Calculating statistics...\n");
    s = statistics(c);
    if(s == NULL) fatal("There is no data from which to generate reports.");
    normalize(c);
    composites(c);
    sortrosters(c, comparename);
    checkfordups(c->roster);
    if(collate) {
            fprintf(stderr, "Dumping collated data...\n");
            writecourse(stdout, c);
            exit(errors ? EXIT_FAILURE : EXIT_SUCCESS);
    }
    sortrosters(c, compare);

    fprintf(stderr, "Producing reports...\n");
    reportparams(stdout, ifile, c);
    if(moments) reportmoments(stdout, s);
    if(composite) reportcomposites(stdout, c, nonames);

    if(freqs) reportfreqs(stdout, s);
    if(quantiles) reportquantiles(stdout, s);
    if(summaries) reportquantilesummaries(stdout, s);
    if(histograms) reporthistos(stdout, c, s);
    if(scores) reportscores(stdout, c, nonames);
    // if(tabsep) reporttabs(stdout, c, nonames);
    if(tabsep) reporttabs(stdout, c);

    fprintf(stderr, "\nProcessing complete.\n");
    printf("%d warning%s issued.\n", warnings+errors,
           warnings+errors == 1? " was": "s were");
    exit(errors ? EXIT_FAILURE : EXIT_SUCCESS);

}

void usage(name)
char *name;
{
    struct option_info *opt;

    fprintf(stderr, "Usage: %s [options] <data file>\n", name);
    fprintf(stderr, "Valid options are:\n");
    for(unsigned int i = 0; i < 14; i++) {
            opt = &option_table[i];
            char optchr[5] = {' ', ' ', ' ', ' ', '\0'};
            if(opt->chr)
              sprintf(optchr, "-%c, ", opt->chr);
            char arg[32];
            if(opt->has_arg)
                sprintf(arg, " <%.10s>", opt->argname);
            else
                sprintf(arg, "%.13s", "");
            fprintf(stderr, "\t%s--%-10s%-13s\t%s\n",
                        optchr, opt->name, arg, opt->descr);
            opt++;
    }
    exit(EXIT_FAILURE);
}
