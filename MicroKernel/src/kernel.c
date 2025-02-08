#include "../MicroKernel/include/kernel.h"

#include <stdlib.h>

#include "../MicroKernel/include/kernelInternals.h"

volatile int g_currentThreadId = 0;
int g_numberOfThreads = 0;
int g_maxNumberOfThread = 0;

Tcb_t *g_tcbs = NULL;
static volatile Tcb_t *g_currentStackPtr = NULL;

static inline __attribute__((always_inline)) void scheduler(void)
{
  do
  {
    g_currentThreadId = (g_currentThreadId + 1) % g_numberOfThreads;
  } while (g_tcbs[g_currentThreadId].isSleeping);  // Skip sleeping threads

  g_currentStackPtr = &g_tcbs[g_currentThreadId];
}

int initKernel(int maxNumberOfThreads)
{
  // Allocate memory for the TCBs.
  g_tcbs = (Tcb_t*)malloc(maxNumberOfThreads * sizeof(Tcb_t));
  if (g_tcbs == NULL)
    return -1;

  // Start from the first thread.
  g_currentStackPtr = &g_tcbs[0];
  g_maxNumberOfThread = maxNumberOfThreads;
  return 0;
}

int initStack(int i, void (*threadFunc)(), int stackSize)
{
  int32_t *tcb_stack = (int32_t*)malloc(stackSize);
  if (tcb_stack == NULL)
	  return -1;

  g_tcbs[i].stackPtr = &tcb_stack[stackSize - 16];     // Stack pointer.
  tcb_stack[stackSize - 1] = (1U << 24);               // T bit in PSR to 1 for thumb mode.
  tcb_stack[stackSize - 2] = (int32_t)(threadFunc);    // Program counter.
  //tcb_stack[stackSize - 3];    // LR
  tcb_stack[stackSize - 4] = 12; // R12
  tcb_stack[stackSize - 5] = 3;  // R3
  tcb_stack[stackSize - 6] = 2;  // R2
  tcb_stack[stackSize - 7] = 1;  // R1
  tcb_stack[stackSize - 8] = i;  // R0, i is index of added thread, it is used of debugging purposes.

  tcb_stack[stackSize - 9] = 11;    // R11
  tcb_stack[stackSize - 10] = 10;   // R10
  tcb_stack[stackSize - 11] = 9;    // R9
  tcb_stack[stackSize - 12] = 8;    // R8
  //tcb_stack[stackSize - 13] = 7;  // R7, It is used by compiler.
  tcb_stack[stackSize - 14] = 6;    // R6
  tcb_stack[stackSize - 15] = 5;    // R5
  tcb_stack[stackSize - 16] = 4;    // R4

  return 0;
}

int addThread(void (*threadFunc)(), int stackSize)
{
  if (g_numberOfThreads >= g_maxNumberOfThread || g_tcbs == NULL)
    return -1;

  __disable_irq();

  int ret = initStack(g_numberOfThreads, threadFunc, stackSize);
  if (ret != 0)
  {
    __enable_irq();
    return -1;
  }

  // Probably these are going to be needed for more sophisticated scheduling policy.
  g_tcbs[g_numberOfThreads].threadId = g_numberOfThreads;
  g_tcbs[g_numberOfThreads].priority = 0;
  g_tcbs[g_numberOfThreads].isSleeping = 0;

  g_numberOfThreads++;
  __enable_irq();
  return 0;
}

int startScheduler(int periodMilliseconds)
{
  if (g_numberOfThreads < 0)
    return -1;

  __disable_irq();

  // Ensure periodMilliseconds is valid
  if (periodMilliseconds <= 0)
    periodMilliseconds = 1;

  uint32_t sysTickFreq = SystemCoreClock / 8;  // SysTick frequency with AHB/8 prescaler
  uint32_t loadValue = (sysTickFreq * periodMilliseconds) / 1000 - 1;

  if (loadValue > 0xFFFFFF)
    loadValue = 0xFFFFFF; // Max allowed value for SysTick LOAD register is 24 bit.

  // Init Systick timer for 100 milliseconds interval
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;    // Enable counter.
  SysTick->LOAD = loadValue;                   // Update Reload for calculated load value.
  SysTick->VAL = 0;                            // Reset counter.
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;   // Enable interrupt.
  SysTick->CTRL &= ~(1 << 2);                  // Systick clock source = AHB / 8 => 100 / 8 = 12 Mhz.
  NVIC_EnableIRQ(SysTick_IRQn);                // Enable interrupt from NVIC.
  NVIC_SetPriority (SysTick_IRQn, 15);         // Set interrupt priority to low.


  // Pendable service call is used to yield the thread.
  NVIC_EnableIRQ(PendSV_IRQn);
  NVIC_SetPriority(PendSV_IRQn, 15);

  __asm("LDR R0, =g_currentStackPtr");  // Load address of current stack pointer to R0.
  __asm("LDR R1, [R0]");                // Load current stack pointer to R2.
  __asm("LDR SP, [R1]");                // Load CPU stack pointer with current stack pointer.
  __asm("POP {R4-R11}");                // Restore registers that are not saved automatically.
  __asm("POP {R0-R3}");                 // Restore other registers.
  __asm("POP {R12}");                   // Restore registers that are not saved automatically.

  // Skip LR and SPR
  __asm("ADD SP, SP, #4");
  __asm("POP {LR}");                 // New start location by popping LR.
  __asm("ADD SP, SP, #4");           // Skip PSR.
  __enable_irq();
  __asm("BX LR");                    // Return from handler. with new stack thus LR with new thread.
}

__attribute__((naked)) void SysTick_Handler(void)
{
  __disable_irq();
  // Save the state of current thread to its stack.
  __asm("PUSH {R4-R11}");                // Save the registers that are not saved automatically.
  __asm("LDR R0, =g_currentStackPtr");   // Get the address of currentPrt.
  __asm("LDR R1, [R0]");                 // Get currentPrt to R1.
  __asm("STR SP, [R1]");                 // Store SP register to R1(actually currentPrt).

  scheduler();                           // Choose next thread and upload its stack to g_currentStackPtr.

  // Load next thread
  __asm("LDR R1, =g_currentStackPtr");  // Address of new stack pointer.
  __asm("LDR R2, [R1]");                // R2 hold new stack pointer.
  __asm("LDR SP, [R2]");                // Update CPU stack pointer.
  __asm("POP {R4-R11}");                // Restore registers that are not get automatically.
  __enable_irq();
  __asm("BX LR");                       // Return from handler. with new stack thus new LR with new thread.
}

void yieldCurrentThread(void)
{
  __disable_irq();
  SysTick->VAL = 0;                      // Reset systick counter because new thread will start already.
  __enable_irq();
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;   // Trigger pendable service that runs scheduler.
}

__attribute__((naked))  void PendSV_Handler(void)
{
  __disable_irq();
  // Save the state of current thread to its stack.
  __asm("PUSH {R4-R11}");                // Save the registers that are not saved automatically.
  __asm("LDR R0, =g_currentStackPtr");   // Get the address of currentPrt.
  __asm("LDR R1, [R0]");                 // Get currentPrt to R1.
  __asm("STR SP, [R1]");                 // Store SP register to R1(actually currentPrt).

  scheduler();                           // Choose next thread and upload its stack to g_currentStackPtr.

  // Load next thread
  __asm("LDR R1, =g_currentStackPtr");  // Address of new stack pointer.
  __asm("LDR R2, [R1]");                // R2 hold new stack pointer.
  __asm("LDR SP, [R2]");                // Update CPU stack pointer.
  __asm("POP {R4-R11}");                // Restore registers that are not get automatically.
  __enable_irq();
  __asm("BX LR");                       // Return from handler. with new stack thus new LR with new thread.
}
