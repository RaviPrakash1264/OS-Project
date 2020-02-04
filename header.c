#define _GNU_SOURCE
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include "ult.h"
#define UNUSED attribute ((unused))
#define TIMERSIG SIGRTMIN
// Environment structure, containing a status and a jump buffer.
typedef struct Env {
int status;
ucontext_t state;
int state_reentered;
int ipc_sender;
int ipc_value;
} Env;
#define NENV 1024
// Status codes for the Env
#define ENV_UNUSED 0
#define ENV_RUNNABLE 1
#define ENV_WAITING 2
#define ENV_STACK_SIZE 16384
static Env envs[NENV];
static int curenv;
void umain(void);
/* provided by user */
/* Define a "successor context" for the purpose of calling env_exit */
static ucontext_t exiter = {0};
/* Preemption timer */
timer_t timer;
const struct itimerspec ts = { {0, 0},
{0, 100000000},
};
static void
make_stack(ucontext_t *ucp)
{
// Reuse existing stack if any
if (ucp->uc_stack.ss_sp)
return;
ucp->uc_stack.ss_size = ENV_STACK_SIZE;
}
int ult_create(ult_entry entry)
{
// Find an available environment
int env;
for (env = 0; (env < NENV); env++) {
if (envs[env].status == ENV_UNUSED) {
// This one is usable!
break; } }
if (env == NENV) {
// No available environments found
return -1; }
envs[env].status = ENV_RUNNABLE;
getcontext(&envs[env].state);
make_stack(&envs[env].state);
envs[env].state.uc_link = &exiter;
makecontext(&envs[env].state, entry, 0);
// Creation worked. Yay.
return env; }
static void ult_schedule(void)
{
int attempts = 0;
int candidate;
while (attempts < NENV)
{
candidate = (curenv + attempts + 1) % NENV;
if (envs[candidate].status == ENV_RUNNABLE)
curenv = candidate;
/* Request delivery of TIMERSIG after 10 ms */
timer_settime(timer, 0, &ts, NULL);
setcontext(&envs[curenv].state);
}
attempts++; }
exit(0); }
void ult_yield(void)
{ envs[curenv].state_reentered = 0;
getcontext(&envs[curenv].state);
if (envs[curenv].state_reentered++ == 0)
{
// Save successful, find the next process to run.
ult_schedule();
}
// We've re-entered. Do nothing.
}
void ult_exit(void)
{
envs[curenv].status = ENV_UNUSED;
ult_schedule();
}
void ult_destroy(int env)
{ envs[env].status = ENV_UNUSED;
}
int ult_getid(void)
{
return curenv;
}
int ult_recv(int *who)
{ envs[curenv].status = ENV_WAITING;
ult_yield();
if (who) *who = envs[curenv].ipc_sender;
return envs[curenv].ipc_value;
}
void ult_send(int toenv, int val)
{
while (envs[toenv].status != ENV_WAITING)
ult_yield();
envs[toenv].ipc_sender = curenv;
envs[toenv].ipc_value = val;
envs[toenv].status = ENV_RUNNABLE; }
static void preempt(int signum UNUSED, siginfo_t *si UNUSED, void
*context UNUSED)
{ ult_yield();
}
static void enable_preemption(void)
{
struct sigaction act = {
.sa_sigaction = preempt, .sa_flags = SA_SIGINFO, };
struct sigevent sigev = { .sigev_notify = SIGEV_SIGNAL, .sigev_signo =
TIMERSIG, .sigev_value.sival_int = 0, };
sigemptyset(&act.sa_mask);
sigaction(TIMERSIG, &act, NULL);
timer_create(CLOCK_PROCESS_CPUTIME_ID, &sigev, &timer);
}
static void initialize_threads(ult_entry new_main) {
curenv = 0;
getcontext(&exiter);
make_stack(&exiter);
makecontext(&exiter, ult_exit, 0);
ult_create(new_main);
setcontext(&envs[curenv].state);
}
int main(int argc UNUSED, char* argv[] UNUSED)
{
enable_preemption();
initialize_threads(umain);
return 0;
}