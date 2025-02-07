#include <stdlib.h>
#include "stm32f4xx.h"
#include "systemClock.h"
#include "kernel.h"
#include "Semaphore.h"
#include "Mutex.h"

#define STACK_SIZE          100
#define MAX_NUM_THREAD      10
#define SCHEDULAR_PERIOD_MS 10

uint32_t g_sharedData = 0;
Mutex_t *g_sharedDataMutex;
Semaphore_t *g_dataAvailableSem;

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

  // Create a mutex for shared data access
  g_sharedDataMutex = Mutex_create();

  // Create a semaphore with initial value 0 (consumer waits) and max value 1.
  g_dataAvailableSem = Semaphore_create(0, 1);

  // Add producer and consumer threads
  addThread(producerThreadFunc, STACK_SIZE);
  addThread(consumerThreadFunc, STACK_SIZE);

  startScheduler(SCHEDULAR_PERIOD_MS);

  return 0; // Code execution never reaches here
}
