#pragma once

#include <stdint.h>

#include "macro.h"

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

#define MSG_TAG_VARIANTS \
  X(NONE, 0, "NONE")     \
  X(SOME, 1, "SOME")     \
  X(QUIT, 2, "QUIT")

enum {
#define X(variant, i, str) MSG_TAG_##variant = (i),
  MSG_TAG_VARIANTS
#undef X
};

typedef struct msgq_message {
  int tag;
  intptr_t value;
} msgq_message;

/// A thread-safe bounded message queue
typedef struct msgq_queue msgq_queue;

///
/// Returns the error message associated with a return code.
///
/// @param rc A return code.
/// @return The error message associated with the given return code, or NULL if the return code is invalid.
///
const char *msgq_failure(int rc);

///
/// Return the tag string associated with a message tag.
///
/// @param tag A message tag.
/// @return The tag string associated with the given tag, or NULL if the tag is invalid.
///
const char *msgq_tag(int tag);

///
/// Creates a new bounded queue with the given capacity.
///
/// Allocates memory for the queue and initializes it.
///
/// @param capacity The maximum number of messages the queue can hold.
/// @return A pointer to a new msgq_queue, or NULL on error.
///
msgq_queue *msgq_create(uint32_t capacity);

///
/// Frees resources associated with the queue.
///
/// Also frees the queue itself.
///
/// Consider using SCOPED_PTR_MessageQueue for scoped cleanup.
///
/// @param queue Message queue.
/// @see msgq_Create()
///
void msgq_destroy(msgq_queue *queue);

DEFINE_TRIVIAL_CLEANUP_FUNC(msgq_queue *, msgq_destroy)
#define SCOPED_PTR_msgq_queue __attribute__((cleanup(msgq_destroyp))) msgq_queue *

///
/// Adds an message to the back of the queue.
///
/// @param queue Message queue.
/// @param in The message to add to the back of the queue.
/// @return 0 if the message was added to the queue, 1 if the queue is full, or a negative value on error.
///
int msgq_put(msgq_queue *queue, msgq_message *in);

///
/// Removes and returns the message at the front of the queue, blocking if the queue is empty.
///
/// @param queue Message queue.
/// @param out The message at the front of the queue.
/// @return 0 if a message was removed from the queue, or a negative value on error.
///
int msgq_get(msgq_queue *queue, msgq_message *out);

///
/// Returns the number of messages in the queue.
///
/// @param queue Message queue.
/// @return The number of messages in the queue.
///
uint32_t msgq_size(msgq_queue *queue);
