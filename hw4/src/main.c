#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "deet.h"
#include "commandFunctions.h"

int dt_argc;
char** dt_argv;

int main(int argc, char *argv[]) {
    // TO BE IMPLEMENTED
    // Remember: Do not put any functions other than main() in this file.

    char* deet_token;
    ssize_t input_length;
    char* lineptr_cpy;
    char* lineptr_cpy_cpy;
    const char* prompt = "deet>";
    const char* delim = " \n";

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);

    int deet_argc;
    char ** deet_argv;

    struct sigaction action;

    action.sa_flags = SA_SIGINFO;
    action.sa_mask = set;
    action.sa_sigaction = sig_handler;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGCHLD, &action, NULL);

    log_startup();

    while(1)
    {
        char* lineptr = "";
        size_t n = 0;
        int num_of_tokens = 0;

        // sigprocmask(SIG_UNBLOCK, &set, NULL);

        log_prompt();

        if(argc > 1)
        {
            int i = 1;
            for(i = 1; i < argc; i++)
            {
                if(strcmp(argv[1], "-p") == 0)  break;
            }

            if(i == argc)
            {
                fprintf(stdout, "%s ", prompt);
                fflush(stdout);
            }
        }
        else
        {
            fprintf(stdout, "%s ", prompt);
            fflush(stdout);
        }

        input_length = getline(&lineptr, &n, stdin);
        // fflush(stdin);
        if(input_length < 0)
        {
            printf("INPUT LENGTH ERROR\n");
            if(errno == EINTR)
            {
                errno = 0;
                fflush(stdin);
                clearerr(stdin);
                // return 0;
                continue;
            }
            else
            {
                clearerr(stdin);
                return 0;
            }
        }

        log_input(lineptr);

        // if(input_length == 1)  continue;

        input_length++;

        lineptr_cpy = malloc(sizeof(char) * input_length);
        lineptr_cpy_cpy = malloc(sizeof(char) * input_length);

        strcpy(lineptr_cpy, lineptr);
        strcpy(lineptr_cpy_cpy, lineptr);
        lineptr_cpy_cpy =strtok(lineptr_cpy_cpy, "\n");

        deet_token = strtok(lineptr, delim);

        while(deet_token != NULL)
        {
            num_of_tokens++;
            deet_token = strtok(NULL, delim);
        }
        num_of_tokens++;

        deet_argv = malloc(sizeof(char*) * num_of_tokens);

        deet_token = strtok(lineptr_cpy, delim);

        int idx = 0;
        while(deet_token != NULL)
        {
            deet_argv[idx] = malloc(sizeof(char) * (strlen(deet_token) + 1));
            strcpy(deet_argv[idx], deet_token);

            deet_token = strtok(NULL, delim);
            idx++;
        }

        if(idx == 0)
        {
            free(lineptr_cpy);
            free(lineptr_cpy_cpy);
            free(deet_argv);
            continue;
        }

        deet_argc = idx;
        deet_argv[deet_argc] = NULL;


        if (strcmp(deet_argv[0], "help") == 0)
        {
            command_help();
        }
        else if (strcmp(deet_argv[0], "quit") == 0 && deet_argc == 1)
        {
            command_quit(deet_argc, deet_argv, lineptr, lineptr_cpy, lineptr_cpy_cpy);
            return 0;
        }
        else if (strcmp(deet_argv[0], "show") == 0 && deet_argc <= 2)
        {
            if(deet_argc == 2)
            {
                int arg_id = atoi(deet_argv[1]);
                command_show2(arg_id);
            }
            else
            {
                command_show1();
            }
        }
        else if (strcmp(deet_argv[0], "run") == 0 && deet_argc >= 2)
        {
            char** d_args = &deet_argv[1];
            int d_argc = deet_argc - 1;

            deet_argc = d_argc;
            dt_argv = d_args;

            pid_t pid = fork();

            sigprocmask(SIG_SETMASK,&set,NULL);
            command_run(pid, d_argc, d_args);
            sigprocmask(SIG_UNBLOCK, &set, NULL);

            // printf("PID: %d\n", getpid());

        }
        else if (strcmp(deet_argv[0], "kill") == 0 && deet_argc == 2)
        {
            int arg_id = atoi(deet_argv[1]);
            command_kill(arg_id);
        }
        else if (strcmp(deet_argv[0], "cont") == 0 && deet_argc == 2)
        {
            int arg_id = atoi(deet_argv[1]);
            command_cont(arg_id);
        }
        else
        {
            log_error(lineptr_cpy_cpy);
            fprintf(stdout, "?\n");
            fflush(stdout);
        }

        for(int i = 0; i < deet_argc; i++)  free(deet_argv[i]);
        free(deet_argv);
        free(lineptr_cpy_cpy);
        free(lineptr_cpy);
        free(lineptr);

    }

    return 0;

    // abort();
}
