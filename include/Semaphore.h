#pragma once
#include "internals.h"

typedef struct Semaphore Semaphore_t;
void Semaphore_init(Semaphore_t *self, int32_t initialValue, uint32_t maxValue);
void Semaphore_release(Semaphore_t *self);
void Semaphore_acquire(Semaphore_t *self);
