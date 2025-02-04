#include <stdlib.h>
#include "stm32f4xx.h"
#include "systemClock.h"
#include "kernel.h"

#define STACK_SIZE          100
#define MAX_NUM_THREAD      10
#define SCHEDULAR_PERIOD_MS 100

uint32_t g_counter[4];
void threadFunc(int32_t index)
{
  while(1)
  {
    g_counter[index]++;
  }
}

int main(void)
{
  initSystemClock();
  initKernel(MAX_NUM_THREAD);

  addThread(threadFunc, STACK_SIZE);
  addThread(threadFunc, STACK_SIZE);
  addThread(threadFunc, STACK_SIZE);
  addThread(threadFunc, STACK_SIZE);
  startScheduler(SCHEDULAR_PERIOD_MS);

  // After scheduler started, rest of the code won't be executed.
  return 0;
}
