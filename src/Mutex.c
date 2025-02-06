#include "Mutex.h"

extern volatile int g_currentThreadId;

struct Mutex
{
  volatile int32_t isLocked;      // 1 if locked, 0 if available
  volatile int32_t ownerThreadId;
};

void Mutex_init(Mutex_t *self)
{
  __disable_irq();
  self->isLocked = 0;        // Unlocked state
  self->ownerThreadId = -1;  // No owner initially
  __enable_irq();
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
