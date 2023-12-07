#include "client_registry.h"
#include "debug.h"
#include "csapp.h"

#include <sys/select.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * A client registry keeps track of the file descriptors for clients
 * that are currently connected.  Each time a client connects,
 * its file descriptor is added to the registry.  When the thread servicing
 * a client is about to terminate, it removes the file descriptor from
 * the registry.  The client registry also provides a function for shutting
 * down all client connections and a function that can be called by a thread
 * that wishes to wait for the client count to drop to zero.  Such a function
 * is useful, for example, in order to achieve clean termination:
 * when termination is desired, the "main" thread will shut down all client
 * connections and then wait for the set of registered file descriptors to
 * become empty before exiting the program.
 */
struct client_registry
{
	int fd_array[FD_SETSIZE];
	sem_t mutex;
	sem_t empty_checker;
	int num_clients;
};

/*
 * Initialize a new client registry.
 *
 * @return  the newly initialized client registry, or NULL if initialization
 * fails.
 */
CLIENT_REGISTRY *creg_init()
{
	CLIENT_REGISTRY *cli_reg = malloc(sizeof(CLIENT_REGISTRY));
	debug("Initialize client registry");
	for(int i = 0; i < FD_SETSIZE; i++)
	{
		cli_reg->fd_array[i] = -1;
	}

	cli_reg->num_clients = 0;

	if(sem_init(&cli_reg->mutex, 0, 1) < 0)	return NULL;
	if(sem_init(&cli_reg->empty_checker, 0, 1) < 0)	return NULL;

	return cli_reg;
}

/*
 * Finalize a client registry, freeing all associated resources.
 * This method should not be called unless there are no currently
 * registered clients.
 *
 * @param cr  The client registry to be finalized, which must not
 * be referenced again.
 */
void creg_fini(CLIENT_REGISTRY *cr)
{
	int client_check = 0;

	for(int i = 0; i < FD_SETSIZE; i++)
	{
		if(cr->fd_array[i] != -1)	client_check = 1;
	}

	if(client_check == 0)
	{
		free(cr);
		sem_destroy(&cr->mutex);
		sem_destroy(&cr->empty_checker);
	}
}

/*
 * Register a client file descriptor.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be registered.
 * @return 0 if registration is successful, otherwise -1.
 */
int creg_register(CLIENT_REGISTRY *cr, int fd)
{
	if(sem_wait(&cr->mutex) < 0)	return -1;

	if(cr->num_clients == 0)	P(&cr->empty_checker);

	cr->fd_array[fd] = fd;
	cr->num_clients++;

	if(sem_post(&cr->mutex) < 0)	return -1;
	debug("Register client fd %d", fd);

	return 0;

}

/*
 * Unregister a client file descriptor, removing it from the registry.
 * If the number of registered clients is now zero, then any threads that
 * are blocked in creg_wait_for_empty() waiting for this situation to occur
 * are allowed to proceed.  It is an error if the CLIENT is not currently
 * registered when this function is called.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be unregistered.
 * @return 0  if unregistration succeeds, otherwise -1.
 */
int creg_unregister(CLIENT_REGISTRY *cr, int fd)
{
	if(sem_wait(&cr->mutex) < 0)	return -1;

	if(cr->num_clients == 1)		V(&cr->empty_checker);

	cr->fd_array[fd] = -1;
	cr->num_clients--;

	if(sem_post(&cr->mutex) < 0)	return -1;
	debug("Unregister client fd %d", fd);

	return 0;
}

/*
 * A thread calling this function will block in the call until
 * the number of registered clients has reached zero, at which
 * point the function will return.  Note that this function may be
 * called concurrently by an arbitrary number of threads.
 *
 * @param cr  The client registry.
 */
void creg_wait_for_empty(CLIENT_REGISTRY *cr)
{
	if(cr->num_clients > 0)		P(&cr->empty_checker);
}

/*
 * Shut down (using shutdown(2)) all the sockets for connections
 * to currently registered clients.  The file descriptors are not
 * unregistered by this function.  It is intended that the file
 * descriptors will be unregistered by the threads servicing their
 * connections, once those server threads have recognized the EOF
 * on the connection that has resulted from the socket shutdown.
 *
 * @param cr  The client registry.
 */
void creg_shutdown_all(CLIENT_REGISTRY *cr)
{
	for(int i = 0; i < FD_SETSIZE; i++)
	{
		if(cr->fd_array[i] != -1)
		{
			shutdown(cr->fd_array[i], SHUT_RDWR);
		}
	}
}