#pragma once

#include "message_queue.h"

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

static const char *const MSGQ_FAILURE_STR[] = {
#define X(variant, i, str) [-(MSGQ_FAILURE_##variant)] = (str),
  MSGQ_FAILURE_VARIANTS
#undef X
};

static const char *const MSG_TAG_STR[] = {
#define X(variant, i, str) [MSG_TAG_##variant] = (str),
  MSG_TAG_VARIANTS
#undef X
};

const char *message_queue_failure(int rc) {
  extern const char *const MSGQ_FAILURE_STR[];

  if (rc > MSGQ_FAILURE_MALLOC || rc < MSGQ_FAILURE_MUTEX_UNLOCK) {
    return NULL;
  }
  return MSGQ_FAILURE_STR[-rc];
}

const char *message_queue_tag(int tag) {
  extern const char *const MSG_TAG_STR[];

  if (tag > MSG_TAG_QUIT || tag < MSG_TAG_NONE) {
    return NULL;
  }
  return MSG_TAG_STR[tag];
}
