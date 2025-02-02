#pragma once

void startScheduler(void);
int addThreads(void (**threadFuncs)(void), int numThreads, int stackSize[]);
