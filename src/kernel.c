#include "kernel.h"
#include <stdint.h>
#include "stm32f4xx.h"

#define NUMBER_OF_THREADS 3
#define STACK_SIZE 100 // 100 * 4 = 400 bytes

typedef struct Tcb
{
  int32_t *stackPtr;
  struct Tcb *nextPtr;
}Tcb_t;

Tcb_t g_tcbs[NUMBER_OF_THREADS];
Tcb_t *g_currentStackPtr;

// Real stack for all threads
int32_t TCB_STACK[NUMBER_OF_THREADS][STACK_SIZE];

void initStack(int i, void (*thread)(void))
{
  g_tcbs[i].stackPtr = &TCB_STACK[i][STACK_SIZE - 16]; // Stack pointer.
  TCB_STACK[i][STACK_SIZE - 1] = (1U << 24);         // T bit in PSR to 1 for thumb mode.
  TCB_STACK[i][STACK_SIZE - 2] = (int32_t)(thread);  // Program counter.
}

void addThreads(void (*thread0)(void), void (*thread1)(void), void (*thread2)(void))
{
  __disable_irq();
  g_tcbs[0].nextPtr = &g_tcbs[1];
  g_tcbs[1].nextPtr = &g_tcbs[2];
  g_tcbs[2].nextPtr = &g_tcbs[0];

  // Init stacks for threads.
  initStack(0, thread0);
  initStack(1, thread1);
  initStack(2, thread2);

  // Start from thread0
  g_currentStackPtr = &g_tcbs[0];
  __enable_irq();
}

void startScheduler(void)
{
  // Init Systick timer for 1ms interval
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;    // Enable counter
  SysTick->LOAD = 12500000;                    // Update Reload for 1 sec
  SysTick->VAL = 0;                            // Reset counter
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;   // Enable interrupt
  SysTick->CTRL &= ~(1 << 2);                  // Systick clock source = AHB / 8 => 100 / 8 = 12 Mhz
  NVIC_EnableIRQ(SysTick_IRQn);                // Enable interrupt from NVIC
  NVIC_SetPriority (SysTick_IRQn, 15);         // Set interrupt priority to low

  __asm("LDR R0, =g_currentStackPtr");  // Load address of current stack pointer to r0
  __asm("LDR R2, [R0]");              // Load current stack pointer to r2
  __asm("LDR SP, [R2]");             // Load CPU stack pointer with current stack pointer
  __asm("POP {R4-R11}");             // Restore registers that are not saved automatically.
  __asm("POP {R12}");                // Restore registers that are not saved automatically.
  __asm("POP {R0-R3}");              // Restore other registers.

  // Skip LR and SPR
  __asm("ADD SP, SP, #4");
  __asm("POP {LR}");                 // New start location by pooping LR
  __asm("ADD SP, SP, #4");           // Skip PSR
  __asm("CPSIE I");                  // Enable interrupt.
  __asm("BX LR");                    // Return from handler. with new stack thus LR with new thread.
}

__attribute__((naked)) void SysTick_Handler(void)
{
  __asm("CPSID I");

  // Save the state of current thread to its stack.
  __asm("PUSH {R4-R11}");              // Save the registers that are not saved automatically
  __asm("LDR R0, =g_currentStackPtr");   // Get the address of currentPrt
  __asm("LDR R1, [R0]");               // Get currentPrt to R1
  __asm("STR SP, [R1]");               // Store SP register to R1(actually currentPrt)

  // Load next thread
  __asm("LDR R1, [R1,#4]");            // Tcb_t->nextPrt offset is 4 bytes.
  __asm("STR R1, [R0]");               // Update currentPtr (Its address already in R0)
  __asm("LDR SP, [R1]");               // Update SP also with new currentPtr
  __asm("POP {R4-R11}");               // Restore registers that are not get automatically.
  __asm("CPSIE I");
  __asm("BX LR");  // Return from handler. with new stack thus new PC with new thread.
}