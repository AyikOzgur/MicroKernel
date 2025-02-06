#include "Semaphore.h"

struct Semaphore
{
  volatile int32_t value;
  uint32_t maxValue;        // Maximum allowed count
};

void Semaphore_init(Semaphore_t *self, int32_t initialValue, uint32_t maxValue)
{
  __disable_irq();
  if (maxValue == 0)
    maxValue = 1;

  if (initialValue > maxValue)
    initialValue = maxValue;

  self->value = initialValue;
  self->maxValue = maxValue;
  __enable_irq();
}

void Semaphore_release(Semaphore_t *self)
{
  __disable_irq();
  if (self->value < self->maxValue)
    self->value++;

  __enable_irq();
}

void Semaphore_acquire(Semaphore_t *self)
{
  while (1)
  {
    // Loop until we can decrement
    __disable_irq();
    if (self->value > 0)
    {
      self->value--;
      __enable_irq();
      return;
    }
    __enable_irq();

    // If no resource is available, yield to let another thread run.
    yieldCurrentThread();
  }
}
