#pragma once
#include "internals.h"

typedef struct Semaphore Semaphore_t;
Semaphore_t* Semaphore_create(int32_t initialValue, uint32_t maxValue);
void Semaphore_destroy(Semaphore_t *self);
void Semaphore_release(Semaphore_t *self);
void Semaphore_acquire(Semaphore_t *self);
