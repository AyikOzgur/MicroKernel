#pragma once
#include "kernel.h"

/**
 * Semaphore structure.
 */
typedef struct Semaphore Semaphore_t;

/**
 * Creates a semaphore.
 * @param initialValue The initial value of the semaphore.
 * @param maxValue The maximum value of the semaphore.
 * @return The semaphore.
 */
Semaphore_t* Semaphore_create(int32_t initialValue, uint32_t maxValue);

/**
 * Destroys a semaphore.
 * @param self The semaphore.
 */
void Semaphore_destroy(Semaphore_t *self);

/**
 * Releases a semaphore.
 * @param self The semaphore.
 */
void Semaphore_release(Semaphore_t *self);

/**
 * Acquires a semaphore.
 * @param self The semaphore.
 */
void Semaphore_acquire(Semaphore_t *self);
