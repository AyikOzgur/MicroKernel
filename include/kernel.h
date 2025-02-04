#pragma once

void startScheduler(int periodMilliseconds);

int initKernel(int maxNumberOfThreads);

int addThread(void (*threadFunc)(), int stackSize);
