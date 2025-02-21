#pragma once
#include <stdint.h>
#include "kernelConfig.h"
#include "stm32f4xx.h"

/**
 * Initializes the kernel.
 * @param maxNumberOfThreads The maximum number of threads that can be created.
 * @return 0 if successful, -1 otherwise.
 */
int initKernel(int maxNumberOfThreads);

/**
 * Adds a thread to the scheduler.
 * @param threadFunc The function to run in the thread.
 * @param stackSize The size of the stack for the thread.
 * @return The thread ID if successful, -1 otherwise.
 */
int addThread(void (*threadFunc)(), int stackSize);

/**
 * Starts the scheduler.
 * @param periodMilliseconds The period of the scheduler in milliseconds.
 * @return 0 if successful, -1 otherwise.
 */
int startScheduler(int periodMilliseconds);

/**
 * Runs the scheduler to switch to the next thread.
 */
void yieldCurrentThread(void);

#ifdef TRACER_ON

/**
 * Set the callback function to send tracer data.
 * @param func The callback function.
 */
void setSendTracerDataCallback(void (*func)(uint8_t*, int));

#endif
