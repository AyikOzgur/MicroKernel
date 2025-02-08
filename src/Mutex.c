#include "Mutex.h"

#include <stdlib.h>

extern volatile int g_currentThreadId;

struct Mutex
{
  volatile int32_t isLocked;      // 1 if locked, 0 if available
  volatile int32_t ownerThreadId;
};

Mutex_t* Mutex_create()
{
  Mutex_t *mutex = (Mutex_t*)malloc(sizeof(Mutex_t));
  if (mutex != NULL)
  {
    mutex->isLocked = 0;        // Unlocked state
    mutex->ownerThreadId = -1;  // No owner initially
  }
  return mutex;
}

void Mutex_destroy(Mutex_t *self)
{
  if (self != NULL)
    free(self);
}

void Mutex_lock(Mutex_t *self)
{
  while (1)
  {
    // Keep trying until successful
    __disable_irq();
    if (self->isLocked == 0)
    {
      // If unlocked, acquire it
      self->isLocked = 1;
      self->ownerThreadId = g_currentThreadId;
      __enable_irq();
      return;
    }
    __enable_irq();
    yieldCurrentThread();  // Mutex is locked, yield execution
  }
}

void Mutex_unlock(Mutex_t *self)
{
  __disable_irq();
  if (self->isLocked == 1 && self->ownerThreadId == g_currentThreadId)
  {
    self->isLocked = 0;        // Unlock mutex
    self->ownerThreadId = -1;  // Clear owner
  }
  __enable_irq();
}
