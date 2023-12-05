#pragma once

#define MSGQ_FAILURE_VARIANTS                      \
  X(MALLOC, -1, "malloc failed")                   \
  X(SEM_CREATE, -2, "Create semaphore failed")     \
  X(SEM_POST, -3, "Post semaphore failed")         \
  X(SEM_TRY_WAIT, -4, "Try-wait semaphore failed") \
  X(SEM_WAIT, -5, "Wait semaphore failed")         \
  X(MUTEX_CREATE, -6, "Create mutex failed")       \
  X(MUTEX_LOCK, -7, "Lock mutex failed ")          \
  X(MUTEX_UNLOCK, -8, "Unlock mutex failed")

enum {
#define X(variant, i, str) MSGQ_FAILURE_##variant = (i),
  MSGQ_FAILURE_VARIANTS
#undef X
};
