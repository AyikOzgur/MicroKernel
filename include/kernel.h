#pragma once
#include "internals.h"

int initKernel(int maxNumberOfThreads);
int addThread(void (*threadFunc)(), int stackSize);
void startScheduler(int periodMilliseconds);
