#include "Mutex.h"
#include "kernelInternals.h"
#include <stdlib.h>

extern Tcb_t *g_tcbs;
extern volatile int g_currentThreadId;
extern int g_numberOfThreads;

struct Mutex
{
  volatile int32_t isLocked;      // 1 if locked, 0 if available
  volatile int32_t ownerThreadId; // Thread that currently owns the mutex
  uint8_t *waitingThreads;        // Array of thread IDs waiting for the mutex
  uint8_t waitingCount;           // Number of waiting threads
};

Mutex_t *Mutex_create()
{
  Mutex_t *mutex = (Mutex_t *)malloc(sizeof(Mutex_t));
  if (mutex != NULL)
  {
    mutex->isLocked = 0;       // Initially unlocked
    mutex->ownerThreadId = -1; // No owner initially
    mutex->waitingCount = 0;
    // Allocate waiting list array sized to maximum number of threads
    mutex->waitingThreads = (uint8_t *)malloc(g_numberOfThreads * sizeof(uint8_t));
    if (mutex->waitingThreads == NULL)
    {
      free(mutex);
      return NULL;
    }
  }
  return mutex;
}

void Mutex_destroy(Mutex_t *self)
{
  if (self == NULL)
    return;

  __disable_irq();
  if (self->waitingThreads != NULL)
  {
    free(self->waitingThreads);
    self->waitingThreads = NULL;
  }
  free(self);
  __enable_irq();
}

void Mutex_lock(Mutex_t *self)
{
  while (1)
  {
    __disable_irq();
    // If mutex is available, acquire it directly.
    if (self->isLocked == 0)
    {
      self->isLocked = 1;
      self->ownerThreadId = g_currentThreadId;
      __enable_irq();
      return;
    }
    // (Optional) If current thread already owns the mutex, simply return.
    if (self->ownerThreadId == g_currentThreadId)
    {
      __enable_irq();
      return;
    }
    // Mutex is locked by another thread:
    // Mark current thread as sleeping and add it to the waiting list.
    g_tcbs[g_currentThreadId].isSleeping = 1;
    self->waitingThreads[self->waitingCount++] = g_currentThreadId;
    __enable_irq();
    yieldCurrentThread();

    // We will wake up from here and in next iteration we will be able to lock and leave
  }
}

void Mutex_unlock(Mutex_t *self)
{
  __disable_irq();
  // Only the owning thread can unlock.
  if (self->isLocked == 1 && self->ownerThreadId == g_currentThreadId)
  {
    if (self->waitingCount > 0)
    {
      // FIFO: remove the first thread from the waiting list.
      uint8_t nextThreadId = self->waitingThreads[0];
      for (int i = 0; i < self->waitingCount - 1; i++)
        self->waitingThreads[i] = self->waitingThreads[i + 1];

      self->waitingCount--;

      // Transfer ownership directly to the waiting thread.
      self->ownerThreadId = nextThreadId;
      // Mutex remains locked but now owned by nextThreadId.
      // Wake up that thread.
      g_tcbs[nextThreadId].isSleeping = 0;
    }
    else
    {
      // No waiting threads; simply unlock.
      self->isLocked = 0;
      self->ownerThreadId = -1;
    }
  }
  __enable_irq();
  yieldCurrentThread();
}
