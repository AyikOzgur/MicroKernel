#include "stm32f4xx.h"
#include "timeBase.h"
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

  timeBase_init();
  addThreads(thread0, thread1, thread2);
  startScheduler();

  while (1)
  {
    i++;
  }
}
