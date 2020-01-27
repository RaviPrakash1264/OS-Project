// Ping-pong a counter between two processes.
// Only need to start one of these -- splits into two with fork.
#include <stdio.h>
#include <ult.h>
void pingpong(void);
void umain(void)
{
int who = ult_create(pingpong);
// get the ball rolling
printf("send 0 from %x to %x\n", ult_getid(), who);
ult_send(who, 0);
pingpong(); }
void pingpong(void)
{
int who;
while (1)
{
int i = ult_recv(&who);
printf("%x got %d from %x\n", ult_getid(), i, who);
if (i == 10)
return;
i++;
ult_send(who, i);
if (i == 10)
return;
} }