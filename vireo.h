/* vireo - Preemptible green threads in C for Linux
 * https://github.com/geofft/vireo
 */

#ifndef VIREO_H
#define VIREO_H

#include <ucontext.h>

// Start point of the green thread
typedef void (*vireo_entry)(void);

// Create (and mark as runnable) a new green thread. Will not be run
// until at least the next call to env_yield(). Returns an identifier
// for the green thread, or -1 if no new green thread can be created.
int vireo_create(vireo_entry entry);

// Yield to the next available green thread, possibly the current one
void vireo_yield(void);

// Indicate that we're done running
void vireo_exit(void);

// Kill another green thread
void vireo_destroy(int id);

// Get this environment's ID
int vireo_getid(void);

// Receive a value from another green thread
int vireo_recv(int *who);

// Send a value to another green thread
void vireo_send(int dest, int val);

#endif
