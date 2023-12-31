#pragma once

#define MSGQ_FAILURE_VARIANTS                        \
    X(NULL_POINTER, -1, "NULL pointer")              \
    X(MALLOC, -2, "malloc failed")                   \
    X(SEM_CREATE, -3, "Create semaphore failed")     \
    X(SEM_POST, -4, "Post semaphore failed")         \
    X(SEM_TRY_WAIT, -5, "Try-wait semaphore failed") \
    X(SEM_WAIT, -6, "Wait semaphore failed")         \
    X(MUTEX_CREATE, -7, "Create mutex failed")       \
    X(MUTEX_LOCK, -8, "Lock mutex failed ")          \
    X(MUTEX_UNLOCK, -9, "Unlock mutex failed")       \
    X(MIN, -10, NULL)

enum {
#define X(variant, i, str) MSGQ_FAILURE_##variant = (i),
    MSGQ_FAILURE_VARIANTS
#undef X
};
