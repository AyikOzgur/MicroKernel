#pragma once
#include "kernel.h"

/**
 * Mutex structure.
 */
typedef struct Mutex Mutex_t;

/**
 * Creates a mutex.
 * @return The mutex.
 */
Mutex_t* Mutex_create();

/**
 * Destroys a mutex.
 * @param self The mutex.
 */
void Mutex_destroy(Mutex_t *self);

/**
 * Locks a mutex.
 * @param self The mutex.
 */
void Mutex_lock(Mutex_t *self);

/**
 * Unlocks a mutex.
 * @param self The mutex.
 */
void Mutex_unlock(Mutex_t *self);
