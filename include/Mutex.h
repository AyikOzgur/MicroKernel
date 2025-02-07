#pragma once
#include "internals.h"

typedef struct Mutex Mutex_t;
Mutex_t* Mutex_create();
void Mutex_destroy(Mutex_t *self);
void Mutex_lock(Mutex_t *self);
void Mutex_unlock(Mutex_t *self);
