// Test preemption by forking off a child process that just spins forever.
// Let it run for a couple time slices, then kill it.
#include <stdio.h>
#include <unistd.h>
#include <ult.h>
void child(void)
{
printf("I am the child. Spinning...\n");
while (1)
/* do nothing */; }
void umain(void) {
int env;
printf("I am the parent. Forking the child...\n");
env = ult_create(child);
printf("I am the parent. Running the child...\n");
ult_yield();
printf("I am the parent. Killing the child...\n");
ult_destroy(env);
}