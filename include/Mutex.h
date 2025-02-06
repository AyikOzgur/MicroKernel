#pragma once
#include "internals.h"

typedef struct Mutex Mutex_t;
void Mutex_init(Mutex_t *self);
void Mutex_lock(Mutex_t *self);
void Mutex_unlock(Mutex_t *self);
