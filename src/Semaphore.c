#include <stdlib.h>
#include "Semaphore.h"
#include "kernelInternals.h"

extern Tcb_t *g_tcbs;
extern int g_currentThreadId;
extern int g_numberOfThreads;

struct Semaphore
{
  volatile int32_t value;
  uint32_t maxValue;                 // Maximum allowed count
  uint8_t *waitingThreads;           // Array of sleeping thread IDs
  uint8_t waitingCount;              // Number of waiting threads
};

Semaphore_t* Semaphore_create(int32_t initialValue, uint32_t maxValue)
{
  if (maxValue == 0)
    maxValue = 1;

  if (initialValue > maxValue)
    initialValue = maxValue;

  Semaphore_t *semaphore = (Semaphore_t*)malloc(sizeof(Semaphore_t));
  if (semaphore == NULL)
    return NULL;

  semaphore->waitingThreads = (uint8_t*)malloc(sizeof(g_numberOfThreads));
  if (semaphore->waitingThreads == NULL)
  {
    free(semaphore);
    return NULL;
  }

  semaphore->value = initialValue;
  semaphore->maxValue = maxValue;
  semaphore->maxValue = maxValue;
  semaphore->waitingCount = 0;
  return semaphore;
}

void Semaphore_destroy(Semaphore_t *self)
{
  if (self == NULL)
    return;

  __disable_irq();

  // Free dynamically allocated waitingThreads array
  if (self->waitingThreads != NULL)
  {
    free(self->waitingThreads);
    self->waitingThreads = NULL;
  }

  free(self);

  __enable_irq();
}

void Semaphore_release(Semaphore_t *self)
{
  __disable_irq();
  if (self->value < self->maxValue)
  {
    self->value++;

    // Wake up all sleeping threads
    while (self->waitingCount > 0)
    {
      uint8_t threadId = self->waitingThreads[--self->waitingCount];  // Remove thread from list
      g_tcbs[threadId].isSleeping = 0;                                // Mark thread as awake
    }
  }
  __enable_irq();

  /* @TODO: Normally without triggering scheduler, anyway other threads should wake up with systick handler scheduler.
            But When we remove manual triggering of scheduler, waiting thread does not wake up. This issue should be
            investigated.
  */
  yieldCurrentThread();  // Immediately trigger scheduler to run.
}

void Semaphore_acquire(Semaphore_t *self)
{
  __disable_irq();
  if (self->value == 0)
  {
    // Put current thread to sleep
    g_tcbs[g_currentThreadId].isSleeping = 1;
    self->waitingThreads[self->waitingCount++] = g_currentThreadId;
    __enable_irq();
    yieldCurrentThread();
  }

  self->value--;
  __enable_irq();
}
