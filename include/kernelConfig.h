#pragma once

/// Lack of this definition will disable tracer feature.
#define TRACER_ON

#ifdef TRACER_ON

/// Tracer buffer item size in terms of scheduler tick.
#define TRACER_BUFFER_SIZE       1023
/// Internal tracker thread stack size in bytes.
#define TRACER_THREAD_STACK_SIZE 800

#endif
