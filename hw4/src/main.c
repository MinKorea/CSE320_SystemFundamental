#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "deet.h"
#include "commandFunctions.h"

int dt_argc;

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
                if(strcmp(argv[i], "-p") == 0)  break;
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
        if(input_length < 0)
        {
            // fprintf(stdout, "GETLINE ERROR\n");
            if(errno == EINTR)
            {
                errno = 0;
                clearerr(stdin);
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
                if(command_show2(arg_id) == 1)
                {
                    log_error(deet_argv[0]);
                    fprintf(stdout, "?\n");
                    fflush(stdout);
                }
            }
            else
            {
                if(command_show1() == 1)
                {
                    log_error(deet_argv[0]);
                    fprintf(stdout, "?\n");
                    fflush(stdout);
                }
            }
        }
        else if (strcmp(deet_argv[0], "run") == 0 && deet_argc >= 2)
        {
            char** d_args = &deet_argv[1];
            int d_argc = deet_argc - 1;

            deet_argc = d_argc;

            pid_t pid = fork();

            sigprocmask(SIG_SETMASK,&set,NULL);
            command_run(pid, d_argc, d_args);
            sigprocmask(SIG_UNBLOCK, &set, NULL);

            // printf("PID: %d\n", getpid());

        }
        else if (strcmp(deet_argv[0], "stop") == 0 && deet_argc == 2)
        {
            int arg_id = atoi(deet_argv[1]);
            if(command_stop(arg_id) == 1)
            {
                log_error(deet_argv[0]);
                fprintf(stdout, "?\n");
                fflush(stdout);
            }
        }
        else if (strcmp(deet_argv[0], "kill") == 0 && deet_argc == 2)
        {
            int arg_id = atoi(deet_argv[1]);
            if(command_kill(arg_id) == 1)
            {
                log_error(deet_argv[0]);
                fprintf(stdout, "?\n");
                fflush(stdout);
            }
        }
        else if (strcmp(deet_argv[0], "release") == 0 && deet_argc == 2)
        {
            int arg_id = atoi(deet_argv[1]);
            if(command_release(arg_id) == 1)
            {
                log_error(deet_argv[0]);
                fprintf(stdout, "?\n");
                fflush(stdout);
            }
        }
        else if (strcmp(deet_argv[0], "cont") == 0 && deet_argc == 2)
        {
            int arg_id = atoi(deet_argv[1]);
            if(command_cont(arg_id) == 1)
            {
                log_error(deet_argv[0]);
                fprintf(stdout, "?\n");
                fflush(stdout);
            }
        }
        else if (strcmp(deet_argv[0], "wait") == 0 && deet_argc <= 3)
        {
            int arg_id = atoi(deet_argv[1]);

            if(deet_argc == 3)
            {
                command_wait2(arg_id, deet_argv[2]);

            }
            else if(deet_argc == 2)
            {
                command_wait1(arg_id);
            }
            else
            {
                log_error(deet_argv[0]);
                fprintf(stdout, "?\n");
                fflush(stdout);
            }
        }
        else if ((strcmp(deet_argv[0], "peek") == 0) && (deet_argc == 3  || deet_argc == 4))
        {
            if(deet_argc == 4)
            {
                int arg_id = atoi(deet_argv[1]);
                size_t addr = strtol(deet_argv[2], NULL, 16);
                int decimal_num = atoi(deet_argv[3]);
                if(command_peek2(arg_id, addr, decimal_num) == 1)
                {
                    log_error(deet_argv[0]);
                    fprintf(stdout, "?\n");
                    fflush(stdout);
                }
            }
            else if(deet_argc == 3)
            {
                int arg_id = atoi(deet_argv[1]);
                size_t addr = strtol(deet_argv[2], NULL, 16);
                if(command_peek1(arg_id, addr) == 1)
                {
                    log_error(deet_argv[0]);
                    fprintf(stdout, "?\n");
                    fflush(stdout);
                }
            }
        }
        else if (strcmp(deet_argv[0], "poke") == 0 && deet_argc == 4)
        {
            int arg_id = atoi(deet_argv[1]);
            size_t addr = strtol(deet_argv[2], NULL, 16);
            long value = strtol(deet_argv[3], NULL, 16);
            if(command_poke(arg_id, addr, value) == 1)
            {
                log_error(deet_argv[0]);
                fprintf(stdout, "?\n");
                fflush(stdout);
            }
        }
        else if ((strcmp(deet_argv[0], "bt") == 0) && (deet_argc == 2  || deet_argc == 3))
        {
            if(deet_argc == 3)
            {
                int arg_id = atoi(deet_argv[1]);
                int decimal_num = atoi(deet_argv[2]);
                if(command_bt2(arg_id, decimal_num) == 1)
                {
                    log_error(deet_argv[0]);
                    fprintf(stdout, "?\n");
                    fflush(stdout);
                }
            }
            else if(deet_argc == 2)
            {
                int arg_id = atoi(deet_argv[1]);
                if(command_bt1(arg_id) == 1)
                {
                    log_error(deet_argv[0]);
                    fprintf(stdout, "?\n");
                    fflush(stdout);
                }
            }
        }
        else
        {
            log_error(deet_argv[0]);
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
}
