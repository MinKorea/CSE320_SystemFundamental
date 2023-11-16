#include "commandFunctions.h"

static int deet_id = 0;
static int num_pids = 0;
static int* pids;
static PSTATE* pstates;
static int d_argc;
static char** d_argv;
static int* pids_argc;
static char*** pids_argv;


void sig_handler(int sig, siginfo_t *sig_info, void* context)
{
	sigset_t set;
    sigemptyset(&set);
    // sigaddset(&set, SIGINT);
    sigaddset(&set, SIGCHLD);

	if(sig == SIGINT)
	{
		log_signal(sig);

		for(int i = 0; i < num_pids; i++)
		{
			sigprocmask(SIG_SETMASK, &set, NULL);
			if(pstates[i] != PSTATE_DEAD)
			{
				command_kill(i);
				sigemptyset(&set);
				sigsuspend(&set);
			}
		}

		log_shutdown();

		exit(0);
	}
	else if(sig == SIGCHLD)
	{
		log_signal(sig);

		if(sig_info->si_code == CLD_EXITED)
		{
            log_state_change(sig_info->si_pid, pstates[deet_id], PSTATE_DEAD, sig_info->si_status);
            pstates[deet_id] = PSTATE_DEAD;
	    	fprintf(stdout, "%d\t%d\tT\tdead\t0x%x\t", deet_id, sig_info->si_pid, sig_info->si_status);

	    	for(int i = 0; i < pids_argc[deet_id]; i++)
			{
				if(i < (pids_argc[deet_id] - 1)) fprintf(stdout, "%s ", pids_argv[deet_id][i]);
				else    		     			 fprintf(stdout, "%s", pids_argv[deet_id][i]);
			}
			fprintf(stdout, "\n");
	    	fflush(stdout);
	    }
	    else if(sig_info->si_code == CLD_KILLED)
	    {
	    	log_state_change(sig_info->si_pid, pstates[deet_id], PSTATE_DEAD, sig_info->si_status);
	    	pstates[deet_id] = PSTATE_DEAD;
	    	fprintf(stdout, "%d\t%d\tT\tdead\t0x%x\t", deet_id, sig_info->si_pid, sig_info->si_status);

	    	for(int i = 0; i < pids_argc[deet_id]; i++)
			{
				if(i < (pids_argc[deet_id] - 1)) fprintf(stdout, "%s ", pids_argv[deet_id][i]);
				else    		     fprintf(stdout, "%s", pids_argv[deet_id][i]);
			}
			fprintf(stdout, "\n");
	    	fflush(stdout);

	    }
	    else if(sig_info->si_code == CLD_STOPPED)
	    {
	    	log_state_change(sig_info->si_pid, PSTATE_RUNNING, PSTATE_STOPPED, sig_info->si_status);

	    	pstates[deet_id] = PSTATE_STOPPED;

	    	fprintf(stdout, "%d\t%d\tT\tstopped\t\t", deet_id, sig_info->si_pid);

	    	for(int i = 0; i < pids_argc[deet_id]; i++)
			{
				if(i < (pids_argc[deet_id] - 1)) fprintf(stdout, "%s ", pids_argv[deet_id][i]);
				else    		     			 fprintf(stdout, "%s", pids_argv[deet_id][i]);
			}

	    	fprintf(stdout, "\n");
	    	fflush(stdout);
	    }
	    else if(sig_info->si_code == CLD_CONTINUED)
	    {
	    	// pstates[deet_id] = PSTATE_RUNNING;
	    	// log_state_change(sig_info->si_pid, pstate, new_state, sig_info->si_status);
	    }
	    else if(sig_info->si_code == CLD_TRAPPED)
	    {
	    	log_state_change(sig_info->si_pid, PSTATE_RUNNING, PSTATE_STOPPED, sig_info->si_status);
	    	fprintf(stdout, "%d\t%d\tT\tstopped\t\t", deet_id, sig_info->si_pid);

	    	pstates[deet_id] = PSTATE_STOPPED;

	    	for(int i = 0; i < pids_argc[deet_id]; i++)
			{
				if(i < (pids_argc[deet_id] - 1)) fprintf(stdout, "%s ", pids_argv[deet_id][i]);
				else    		     			 fprintf(stdout, "%s", pids_argv[deet_id][i]);
			}

	    	fprintf(stdout, "\n");
	    	fflush(stdout);
	    }
	    else
	    {
	    	fprintf(stdout, "PID: %d\n", getpid());
	    	fprintf(stdout, "CLD_EXITED: %d\n", CLD_EXITED);
	    	fprintf(stdout, "CLD_KILLED: %d\n", CLD_KILLED);
	    	fprintf(stdout, "CLD_STOPPED: %d\n", CLD_STOPPED);
	    	fprintf(stdout, "CLD_CONTINUED: %d\n", CLD_CONTINUED);
	    	fprintf(stdout, "SI_CODE: %d\n", sig_info->si_code);
	    }
	}
}

void command_help()
{
	fprintf(stdout, "Available commands:\nhelp -- Print this help message\nquit (<=0 args) -- Quit the program\nshow (<=1 args) -- Show process info\nrun (>=1 args) -- Start a process\nstop (1 args) -- Stop a running process\ncont (1 args) -- Continue a stopped process\nrelease (1 args) -- Stop tracing a process, allowing it to continue normally\nwait (1-2 args) -- Wait for a process to enter a specified state\nkill (1 args) -- Forcibly terminate a process\npeek (2-3 args) -- Read from the address space of a traced process\npoke (3 args) -- Write to the address space of a traced process\nbt (1 args) -- Show a stack trace for a traced process\n");
}

void command_quit(int deet_argc, char** deet_argv,
 char* ptr1, char* ptr2, char* ptr3)
{
	sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);

	for(int i = 0; i < num_pids; i++)
	{
		if(pstates[i] != PSTATE_DEAD)
		{
			sigprocmask(SIG_SETMASK, &set, NULL);
			command_kill(i);
			sigemptyset(&set);
			sigsuspend(&set);
		}
	}

	log_shutdown();

	free(pids_argv);
	free(d_argv);
	for(int i = 0; i < deet_argc; i++)  free(deet_argv[i]);
    free(deet_argv);
    free(ptr3);
    free(ptr2);
    free(ptr1);
}

void command_show1()
{
	for(int i = 0; i < num_pids; i++)
	{
		if(pstates[i] == PSTATE_DEAD)
		{
			fprintf(stdout, "%d\t%d\tT\tdead\t\t", i, pids[i]);
	    	for(int j = 0; j < pids_argc[i]; j++)
			{
				if(j < (pids_argc[i] - 1)) 		 fprintf(stdout, "%s ", pids_argv[i][j]);
				else    		     			 fprintf(stdout, "%s", pids_argv[i][j]);
			}
	    	fprintf(stdout, "\n");
		}
		else if(pstates[i] == PSTATE_RUNNING)
		{
			fprintf(stdout, "%d\t%d\tT\trunning\t\t", i, pids[i]);
	    	for(int j = 0; j < pids_argc[i]; j++)
			{
				if(j < (pids_argc[i] - 1)) 		 fprintf(stdout, "%s ", pids_argv[i][j]);
				else    		     			 fprintf(stdout, "%s", pids_argv[i][j]);
			}
	    	fprintf(stdout, "\n");
		}
		else if(pstates[i] == PSTATE_STOPPING)
		{
			fprintf(stdout, "%d\t%d\tT\tstopping\t\t", i, pids[i]);
	    	for(int j = 0; j < pids_argc[i]; j++)
			{
				if(j < (pids_argc[i] - 1)) 		 fprintf(stdout, "%s ", pids_argv[i][j]);
				else    		     			 fprintf(stdout, "%s", pids_argv[i][j]);
			}
	    	fprintf(stdout, "\n");
		}
		else if(pstates[i] == PSTATE_STOPPED)
		{
			fprintf(stdout, "%d\t%d\tT\tstopped\t\t", i, pids[i]);
	    	for(int j = 0; j < pids_argc[i]; j++)
			{
				if(j < (pids_argc[i] - 1)) 		 fprintf(stdout, "%s ", pids_argv[i][j]);
				else    		     			 fprintf(stdout, "%s", pids_argv[i][j]);
			}
	    	fprintf(stdout, "\n");
		}
		else if(pstates[i] == PSTATE_CONTINUING)
		{
			fprintf(stdout, "%d\t%d\tT\tcontinuing\t\t", i, pids[i]);
	    	for(int j = 0; j < pids_argc[i]; j++)
			{
				if(j < (pids_argc[i] - 1)) 		 fprintf(stdout, "%s ", pids_argv[i][j]);
				else    		     			 fprintf(stdout, "%s", pids_argv[i][j]);
			}
	    	fprintf(stdout, "\n");
		}
		else if(pstates[i] == PSTATE_KILLED)
		{
			fprintf(stdout, "%d\t%d\tT\tkilled\t\t", i, pids[i]);
	    	for(int j = 0; j < pids_argc[i]; j++)
			{
				if(j < (pids_argc[i] - 1)) 		 fprintf(stdout, "%s ", pids_argv[i][j]);
				else    		     			 fprintf(stdout, "%s", pids_argv[i][j]);
			}
	    	fprintf(stdout, "\n");
		}
	}
}

void command_show2(int d_id)
{
	if(pstates[d_id] == PSTATE_DEAD)
	{
		fprintf(stdout, "%d\t%d\tT\tdead\t\t", d_id, pids[d_id]);
    	for(int j = 0; j < pids_argc[d_id]; j++)
		{
			if(j < (pids_argc[d_id] - 1)) 		 fprintf(stdout, "%s ", pids_argv[d_id][j]);
			else    		     			 	 fprintf(stdout, "%s", pids_argv[d_id][j]);
		}
    	fprintf(stdout, "\n");
	}
	else if(pstates[d_id] == PSTATE_RUNNING)
	{
		fprintf(stdout, "%d\t%d\tT\trunning\t\t", d_id, pids[d_id]);
    	for(int j = 0; j < pids_argc[d_id]; j++)
		{
			if(j < (pids_argc[d_id] - 1)) 		 fprintf(stdout, "%s ", pids_argv[d_id][j]);
			else    		     			 	 fprintf(stdout, "%s", pids_argv[d_id][j]);
		}
    	fprintf(stdout, "\n");
	}
	else if(pstates[d_id] == PSTATE_STOPPING)
	{
		fprintf(stdout, "%d\t%d\tT\tstopping\t\t", d_id, pids[d_id]);
    	for(int j = 0; j < pids_argc[d_id]; j++)
		{
			if(j < (pids_argc[d_id] - 1)) 		 fprintf(stdout, "%s ", pids_argv[d_id][j]);
			else    		     			 	 fprintf(stdout, "%s", pids_argv[d_id][j]);
		}
    	fprintf(stdout, "\n");
	}
	else if(pstates[d_id] == PSTATE_STOPPED)
	{
		fprintf(stdout, "%d\t%d\tT\tstopped\t\t", d_id, pids[d_id]);
    	for(int j = 0; j < pids_argc[d_id]; j++)
		{
			if(j < (pids_argc[d_id] - 1)) 		 fprintf(stdout, "%s ", pids_argv[d_id][j]);
			else    		     			 fprintf(stdout, "%s", pids_argv[d_id][j]);
		}
    	fprintf(stdout, "\n");
	}
	else if(pstates[d_id] == PSTATE_CONTINUING)
	{
		fprintf(stdout, "%d\t%d\tT\tcontinuing\t\t", d_id, pids[d_id]);
    	for(int j = 0; j < pids_argc[d_id]; j++)
		{
			if(j < (pids_argc[d_id] - 1)) 		 fprintf(stdout, "%s ", pids_argv[d_id][j]);
			else    		     			 	 fprintf(stdout, "%s", pids_argv[d_id][j]);
		}
    	fprintf(stdout, "\n");
	}
	else if(pstates[d_id] == PSTATE_KILLED)
	{
		fprintf(stdout, "%d\t%d\tT\tkilled\t\t", d_id, pids[d_id]);
    	for(int j = 0; j < pids_argc[d_id]; j++)
		{
			if(j < (pids_argc[d_id] - 1)) 		 fprintf(stdout, "%s ", pids_argv[d_id][j]);
			else    		     			 	 fprintf(stdout, "%s", pids_argv[d_id][j]);
		}
    	fprintf(stdout, "\n");
	}
}

void command_run(pid_t pid, int argc, char** argv)
{
	int status = 0;
	// sigset_t set;
    // sigemptyset(&set);
    // sigaddset(&set, SIGCHLD);

    d_argc = argc;
    d_argv = malloc(sizeof(char*) * d_argc);

    for(int i = 0; i < d_argc; i++)
    {
    	d_argv[i] = malloc(sizeof(char) * strlen(argv[i]) + 1);
    	d_argv[i] = memcpy(d_argv[i], argv[i], strlen(argv[i]) + 1);
    }

	if(pid == 0)
	{
		// dup2(STDOUT_FILENO,STDERR_FILENO);
		// dup2(STDERR_FILENO,STDOUT_FILENO);

		int i;
		for(i = 0; i < num_pids; i++)
		{
			if(pstates[i] == PSTATE_DEAD)
			{
				deet_id = i;
				log_state_change(getpid(), PSTATE_DEAD, PSTATE_NONE, status);
				break;
			}
		}

		if(i == num_pids)	deet_id = num_pids;

		log_state_change(getpid(), PSTATE_NONE, PSTATE_RUNNING, status);
		fprintf(stdout, "%d\t%d\tT\trunning\t\t", deet_id, getpid());
		for(int i = 0; i < argc; i++)
		{
			if(i < (argc - 1)) fprintf(stdout, "%s ", argv[i]);
			else    		   fprintf(stdout, "%s", argv[i]);
		}

		fprintf(stdout, "\n");
		fflush(stdout);

		dup2(STDERR_FILENO,STDOUT_FILENO);
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);

		if(execvp(argv[0], argv) == -1)
		{
			// printf("ERRNO_BEFORE: %d\n", errno);
			// errno = 0x100;
			// printf("ERRNO_AFTER: %d\n", errno);
			exit(errno);
		}
	}
	else if(pid > 0)
	{
		waitpid(pid, &status, WUNTRACED);

		int i;
		for(i = 0; i < num_pids; i++)
		{
			if(pstates[i] == PSTATE_DEAD)
			{
				pids[i] = pid;
				pids_argc[i] = d_argc;
				pids_argv[i] = realloc(pids_argv[i], sizeof(char*) * (d_argc + 1));
				for(int j = 0; j < d_argc; j++)
				{
					pids_argv[i][j] = realloc(pids_argv[i][j], sizeof(char) * (strlen(d_argv[j]) + 1));
	    			pids_argv[i][j] = memcpy(pids_argv[i][j], d_argv[j], strlen(d_argv[j]) + 1);
				}
				deet_id = i;
				// fprintf(stdout, "ARGS: %s\n", pids_argv[i][0]);
				break;
			}
		}

		if(num_pids == 0)
		{
			num_pids++;
			pids = malloc(sizeof(int) * num_pids);
			pstates = malloc(sizeof(PSTATE) * num_pids);
			pids_argc = malloc(sizeof(int) * num_pids);

			pids_argv = malloc(sizeof(char**) * (num_pids + 1));
			pids_argv[0] = malloc(sizeof(char*) * (d_argc + 1));
			for(int i = 0; i < d_argc; i++)
			{
				pids_argv[0][i] = malloc(sizeof(char) * strlen(d_argv[i]) + 1);
    			pids_argv[0][i] = memcpy(pids_argv[0][i], d_argv[i], strlen(d_argv[i]) + 1);
			}

			pids_argc[0] = d_argc;
			pids[0] = pid;
			deet_id = 0;
			// fprintf(stdout, "ARGS: %s\n", pids_argv[0][0]);
		}
		else if(i == num_pids)
		{
			num_pids++;
			pids = realloc(pids, sizeof(char*) * num_pids);
			pstates = realloc(pstates, sizeof(PSTATE) * num_pids);
			pids_argc = realloc(pids_argc, sizeof(int) * num_pids);

			pids_argv = realloc(pids_argv, sizeof(char*) * (num_pids + 1));
			pids_argv[num_pids-1] = malloc(sizeof(char*) * (d_argc + 1));
			for(int i = 0; i < d_argc; i++)
			{
				pids_argv[num_pids-1][i] = malloc(sizeof(char) * (strlen(d_argv[i]) + 1));
    			pids_argv[num_pids-1][i] = memcpy(pids_argv[num_pids-1][i], d_argv[i], strlen(d_argv[i]) + 1);
			}

			pids_argc[num_pids - 1] = d_argc;
			pids[num_pids - 1] = pid;
			deet_id = num_pids - 1;
			// fprintf(stdout, "ARGS: %s\n", pids_argv[i][0]);
		}

		// for(int j = 0; j < num_pids; j++)
		// {
		// 	printf("deet_id: %d\n", j);
		// 	printf("PID[%d]: %d", j, pids[j]);
		// }
	}
}

void command_cont(int d_id)
{
	int status = 0;

	deet_id = d_id;

	log_state_change(pids[deet_id], pstates[deet_id], PSTATE_RUNNING, status);
	pstates[deet_id] = PSTATE_RUNNING;

	fprintf(stdout, "%d\t%d\tT\trunning\t\t", deet_id, pids[deet_id]);

	for(int i = 0; i < pids_argc[deet_id]; i++)
	{
		if(i < (pids_argc[deet_id] - 1)) fprintf(stdout, "%s ", pids_argv[deet_id][i]);
		else    		  			     fprintf(stdout, "%s", pids_argv[deet_id][i]);
	}
	fprintf(stdout, "\n");
	fflush(stdout);

	ptrace(PTRACE_CONT, pids[deet_id], 1, 0);
}

void command_kill(int d_id)
{
	int status = 0;
	deet_id = d_id;

	log_state_change(pids[d_id], pstates[d_id], PSTATE_KILLED, status);
	pstates[deet_id] = PSTATE_KILLED;

	fprintf(stdout, "%d\t%d\tT\tkilled\t\t", d_id, pids[d_id]);

	for(int i = 0; i < pids_argc[d_id]; i++)
	{
		if(i < (pids_argc[d_id] - 1)) fprintf(stdout, "%s ", pids_argv[d_id][i]);
		else    		  			  fprintf(stdout, "%s", pids_argv[d_id][i]);
	}
	fprintf(stdout, "\n");
	fflush(stdout);

	kill(pids[d_id], SIGKILL);
}
