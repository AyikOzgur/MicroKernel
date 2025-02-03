#pragma once

void startScheduler(void);
int addThreads(void (**threadFuncs)(), int numThreads, int stackSize[]);
