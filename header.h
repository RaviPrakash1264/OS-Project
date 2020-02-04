#ifndef ULT_H
#define ULT_H
#include <ucontext.h>
// Start point of the thread
typedef void (*ult_entry)(void);
// Create (and mark as runnable) a new thread. Will not be run
// until at least the next call to env_yield(). Returns an identifier
// for the thread, or -1 if no new thread can be created.
int ult_create(ult_entry entry);
// Yield to the next available green thread, possibly the current one
void ult_yield(void);
// Indicate that we're done running
void ult_exit(void);
// Kill another green thread
void ult_destroy(int id);
// Get this environment's ID
int ult_getid(void);
// Receive a value from another green thread
int ult_recv(int *who);
// Send a value to another green thread
void ult_send(int dest, int val);
#endif