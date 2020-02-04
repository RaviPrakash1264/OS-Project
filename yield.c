#include <stdio.h>
#include <ult.h>
static void yield_thread(void)
{
int i;
printf("Hello, I am environment %08x.\n", ult_getid());
for (i = 0; i < 5; i++)
{
ult_yield();
printf("Back in environment %08x, iteration %d.\n", ult_getid(), i);
}
printf("All done in environment %08x.\n", ult_getid());
}
void umain(void)
{
int i;
for (i = 0; i < 3; i++)
{
ult_create(yield_thread);
}
ult_exit();
}