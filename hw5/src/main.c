#include "debug.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"
#include "server.h"
#include "csapp.h"

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

static void terminate(int status);

CLIENT_REGISTRY *client_registry;
int listenfd, *connfdp;

void sig_handler(int signo, siginfo_t *info, void* context);

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    char* port;

    for(int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i], "-p") == 0)
        {
            port = argv[i+1];
        }
    }

    // Perform required initializations of the client_registry,
    // transaction manager, and object store.
    client_registry = creg_init();
    trans_init();
    store_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function xacto_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    struct sigaction action;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);

    action.sa_flags = SA_SIGINFO;
    action.sa_mask = set;
    action.sa_sigaction = sig_handler;
    sigaction(SIGHUP, &action, NULL);

    
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    listenfd = Open_listenfd(port);

    while (1)
    {
        clientlen=sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd,
        (SA *) &clientaddr, &clientlen);
        Pthread_create(&tid, NULL, xacto_client_service, connfdp);
    }


    fprintf(stderr, "You have to finish implementing main() "
	    "before the Xacto server will function.\n");

    terminate(EXIT_SUCCESS);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    debug("Wait ing for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    trans_fini();
    store_fini();

    debug("Xacto server terminating");

    shutdown(listenfd, SHUT_RD);
    close(listenfd);
    free(connfdp);

    exit(status);
}

void sig_handler(int signo, siginfo_t *info, void* context)
{
        if(signo == SIGHUP)
        {
            terminate(info->si_status);
        }
}