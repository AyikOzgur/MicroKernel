#include <stdlib.h>
#include <systemClock.h>
#include "stm32f4xx.h"
#include "../MicroKernel/include/kernel.h"
#include "../MicroKernel/include/Mutex.h"
#include "../MicroKernel/include/Semaphore.h"

#define THREAD_STACK_SIZE_BYTES  100
#define MAX_NUM_THREAD           10
#define SCHEDULAR_PERIOD_MS      10

uint32_t g_sharedData = 0;
Mutex_t *g_sharedDataMutex = NULL;
Semaphore_t *g_dataAvailableSem = NULL;

void producerThreadFunc()
{
  while (1)
  {
    Mutex_lock(g_sharedDataMutex);
    g_sharedData++;
    Mutex_unlock(g_sharedDataMutex);

    // Signal that new data is available
    Semaphore_release(g_dataAvailableSem);
  }
}

void consumerThreadFunc()
{
  uint32_t readValue = 0;
  while (1)
  {
    // Wait until data is available
    Semaphore_acquire(g_dataAvailableSem);

    Mutex_lock(g_sharedDataMutex);
    readValue = g_sharedData;
    Mutex_unlock(g_sharedDataMutex);
  }
}

int main(void)
{
  initSystemClock();

  initKernel(MAX_NUM_THREAD);

  g_sharedDataMutex = Mutex_create();

  g_dataAvailableSem = Semaphore_create(0, 1);

  addThread(producerThreadFunc, THREAD_STACK_SIZE_BYTES);
  addThread(consumerThreadFunc, THREAD_STACK_SIZE_BYTES);

  startScheduler(SCHEDULAR_PERIOD_MS);

  return 0; // Code execution never reaches here
}
