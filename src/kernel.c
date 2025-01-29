#include "kernel.h"
#include <stdint.h>

#define NUMBER_OF_THREADS 3
#define STACK_SIZE 100

typedef struct Tcb
{
   int32_t *stackPtr;
   struct Tcb *nextPtr;
}Tcb_t;

Tcb_t tcbs[NUMBER_OF_THREADS];
Tcb_t *currentStackPtr;

// Real stack for all threads
int32_t TCB_STACK[NUMBER_OF_THREADS][STACK_SIZE];

void initStack(int i)
{
  tcbs[i].stackPtr = &TCB_STACK[i][STACK_SIZE - 16]; // Stack pointer.
  TCB_STACK[i][STACK_SIZE - 1] = (1U << 21);         // T bit in PSR to 1 for thumb mode.

}

void addThreads(void (*thread0), void (*thread1), void (*thread2))
{
  __disable_irq();
  tcbs[0].nextPtr = &tcbs[1];
  tcbs[1].nextPtr = &tcbs[2];
  tcbs[2].nextPtr = &tcbs[0];

  // Init stack of thread0
  initStack(0);
  TCB_STACK[0][STACK_SIZE - 2] = (int32_t)(thread0);

  // Init stack of thread1
  initStack(1);
  TCB_STACK[1][STACK_SIZE - 2] = (int32_t)(thread1);

  // Init stack of thread2
  initStack(2);
  TCB_STACK[2][STACK_SIZE - 2] = (int32_t)(thread2);

  // Start from thread0
  currentStackPtr = &tcbs[0];

  __enable_irq();
}

void startScheduler(void)
{
  /// @TODO : Implement starting scheduler with first thread.
}

__attribute__((naked)) void SysTick_Handler(void)
{
  __disable_irq();

  // Save the state of current thread to its stack.
  __asm("PUSH {R4-R11}");             // Save the registers that are not saved automatically
  __asm("LDR R0 =currentStackPtr");   // Get the address of currentPrt
  __asm("LDR R1,[R0]");               // Get currentPrt to R1
  __asm("STR SP,[R1]");               // Store SP register to R1(actually currentPrt)

  // Load next thread
  __asm("LDR R1,[R1,#4]");            // Tcb_t->nextPrt offset is 4 bytes.
  __asm("STR R1,[R0]");               // Update currentPtr (Its address already in R0)
  __asm("LDR SP,[R1]");               // Update SP also with new currentPtr
  __asm("POP {R4-R11}");              // Restore registers that are not saved automatically.

  __enable_irq();

  __asm("BX LR");  // Return from handler. with new stack thus new PC with new thread.
}
