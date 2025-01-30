#pragma once

void startScheduler(void);
void addThreads(void (*thread0)(void), void (*thread1)(void), void (*thread2)(void));
