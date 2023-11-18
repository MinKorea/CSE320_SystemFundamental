#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>


#include "deet.h"

void sig_handler(int sig, siginfo_t *sig_info, void* context);
void command_help();
void command_quit(int deet_argc, char** deet_argv, char* ptr1, char* ptr2, char* ptr3);
int command_show1();
int command_show2(int d_id);
void command_run(pid_t pid, int argc, char** argv);
int command_stop(int d_id);
int command_cont(int d_id);
int command_release(int d_id);
int command_wait1(int d_id);
int command_wait2(int d_id, char* arg);
int command_kill(int d_id);




