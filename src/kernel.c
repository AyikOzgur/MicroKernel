#include "kernel.h"
#include <stdint.h>
#include <stdlib.h>
#include "stm32f4xx.h"


typedef struct Tcb
{
  int32_t *stackPtr;
  struct Tcb *nextPtr;
}Tcb_t;

Tcb_t *g_tcbs;
Tcb_t *g_currentStackPtr;

int initStack(int i, void (*thread)(), int stackSize)
{
  int32_t *tcb_stack = (int32_t*)malloc(stackSize);
  if (tcb_stack == NULL)
	  return -1;
  g_tcbs[i].stackPtr = &tcb_stack[stackSize - 16]; // Stack pointer.
  tcb_stack[stackSize - 1] = (1U << 24);           // T bit in PSR to 1 for thumb mode.
  tcb_stack[stackSize - 2] = (int32_t)(thread);    // Program counter.
  //tcb_stack[stackSize - 3];    // LR
  tcb_stack[stackSize - 4] = 12; // R12
  tcb_stack[stackSize - 5] = 3;  // R3
  tcb_stack[stackSize - 6] = 2;  // R2
  tcb_stack[stackSize - 7] = 1;  // R1
  tcb_stack[stackSize - 8] = 0;  // R0

  tcb_stack[stackSize - 9] = 11;    // R11
  tcb_stack[stackSize - 10] = 10;   // R10
  tcb_stack[stackSize - 11] = 9;    // R9
  tcb_stack[stackSize - 12] = 8;    // R8
  tcb_stack[stackSize - 13] = 155;  // R7, is wrong
  tcb_stack[stackSize - 14] = 6;    // R6
  tcb_stack[stackSize - 15] = 5;    // R5
  tcb_stack[stackSize - 16] = 4;    // R4

  /// @TODO : R7 value is wrong in debugger. Probably compiler is using R7 for something else.

  return 0;
}

int addThreads(void (**threadFuncs)(), int numThreads, int stackSize[])
{
  if (numThreads <= 0)
    return -1;

  __disable_irq();

  // Allocate memory for the TCBs
  g_tcbs = (Tcb_t*)malloc(numThreads * sizeof(Tcb_t));
  if (g_tcbs == NULL)
  {
    __enable_irq();
    return -1;
  }

  // Link TCBs in a circular manner
  for (int i = 0; i < numThreads; i++)
  {
    g_tcbs[i].nextPtr = &g_tcbs[(i + 1) % numThreads]; // Circular scheduling.
    int ret = initStack(i, threadFuncs[i], stackSize[i]);
    if (ret != 0)
    {
      __enable_irq();
      return -1;
    }
  }

  // Start from the first thread
  g_currentStackPtr = &g_tcbs[0];

  __enable_irq();

  return 0;
}

void startScheduler(void)
{
  __disable_irq();

  // Init Systick timer for 1 sec interval
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;    // Enable counter
  SysTick->LOAD = 12500000;                    // Update Reload for 1 sec
  SysTick->VAL = 0;                            // Reset counter
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;   // Enable interrupt
  SysTick->CTRL &= ~(1 << 2);                  // Systick clock source = AHB / 8 => 100 / 8 = 12 Mhz
  NVIC_EnableIRQ(SysTick_IRQn);                // Enable interrupt from NVIC
  NVIC_SetPriority (SysTick_IRQn, 15);         // Set interrupt priority to low

  __asm("LDR R0, =g_currentStackPtr");  // Load address of current stack pointer to r0
  __asm("LDR R1, [R0]");                // Load current stack pointer to r2
  __asm("LDR SP, [R1]");                // Load CPU stack pointer with current stack pointer
  __asm("POP {R4-R11}");                // Restore registers that are not saved automatically.
  __asm("POP {R0-R3}");                 // Restore other registers.
  __asm("POP {R12}");                   // Restore registers that are not saved automatically.

  // Skip LR and SPR
  __asm("ADD SP, SP, #4");
  __asm("POP {LR}");                 // New start location by poping LR
  __asm("ADD SP, SP, #4");           // Skip PSR
  __enable_irq();
  __asm("BX LR");                    // Return from handler. with new stack thus LR with new thread.
}

__attribute__((naked)) void SysTick_Handler(void)
{
  __disable_irq();
  // Save the state of current thread to its stack.
  __asm("PUSH {R4-R11}");                // Save the registers that are not saved automatically
  __asm("LDR R0, =g_currentStackPtr");   // Get the address of currentPrt
  __asm("LDR R1, [R0]");                 // Get currentPrt to R1
  __asm("STR SP, [R1]");                 // Store SP register to R1(actually currentPrt)

  // Load next thread
  __asm("LDR R1, [R1,#4]");            // Tcb_t->nextPrt offset is 4 bytes.
  __asm("STR R1, [R0]");               // Update currentPtr (Its address already in R0)
  __asm("LDR SP, [R1]");               // Update SP also with new currentPtr
  __asm("POP {R4-R11}");               // Restore registers that are not get automatically.
  __enable_irq();
  __asm("BX LR");                      // Return from handler. with new stack thus new LR with new thread.
}
