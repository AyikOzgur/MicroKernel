#pragma once
#include "kernelConfig.h"

/**
 * Thread control block (TCB) structure.
 */
typedef struct
{
  int32_t *stackPtr;
  uint8_t threadId;  // Used for future scheduling policy
  uint8_t priority;  // Used for future scheduling policy
  uint8_t isSleeping; // 1 = Thread is blocked, 0 = Running
} Tcb_t;


#ifdef TRACER_ON

/**
 * Trace event structure.
 */
typedef struct __attribute__((packed))
{
  uint16_t deltaTime : 10;  // 10 bits for delta time
  uint16_t eventType : 2;   // 2 bits for event type
  uint16_t threadId  : 4;   // 4 bits for thread ID
} TraceEvent_t;

/**
 * Trace event type enumeration.
 */
typedef enum
{
  TRACE_EVENT_SYSTICK = 0, // Scheduler tick event (e.g., after SysTick)
  TRACE_EVENT_PENDSV,      // Task yield event (e.g., after PendSV)
  TRACE_EVENT_IDLE,        // Idle event (if you want to log idle periods)
  TRACE_EVENT_OTHER        // Reserved for future use or additional events
} TraceEventType;

#endif
