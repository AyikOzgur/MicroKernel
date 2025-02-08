#pragma once

typedef struct
{
  int32_t *stackPtr;
  uint8_t threadId;  // Used for future scheduling policy
  uint8_t priority;  // Used for future scheduling policy
  uint8_t isSleeping; // 1 = Thread is blocked, 0 = Running
} Tcb_t;
