#pragma once
#include "internals.h"

int initKernel(int maxNumberOfThreads);
int addThread(void (*threadFunc)(), int stackSize);
int startScheduler(int periodMilliseconds);
