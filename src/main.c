#include <stdlib.h>
#include "stm32f4xx.h"
#include "systemClock.h"
#include "kernel.h"

uint32_t g_counter0 = 0;
uint32_t g_counter1 = 0;
uint32_t g_counter2 = 0;

void thread0(void)
{
  while(1)
  {
    g_counter0++;
  }
}

void thread1(void)
{
  while(1)
  {
    g_counter1++;
  }
}

void thread2(void)
{
  while(1)
  {
    g_counter2++;
  }
}


int main(void)
{
  int i = 0;

  initSystemClock();

  void (*threads[])(void) = {thread0, thread1, thread2};
  int numThreads = sizeof(threads) / sizeof(threads[0]);

  int stackSize = 100;
  int *stackSizes = (int*)malloc(sizeof(int) * numThreads);
  for (int i = 0; i < numThreads; i++)
    stackSizes[i] = stackSize;

  if (addThreads(threads, numThreads, stackSizes) == 0)
    startScheduler();

  while (1)
  {
    i++;
  }
}
