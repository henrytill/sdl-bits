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

typedef enum MessageQueueFailure MessageQueueFailure;
enum MessageQueueFailure {
#define X(variant, i, str) MSGQ_FAILURE_##variant = i,
  MSGQ_FAILURE_VARIANTS
#undef X
};

#define MSG_TAG_VARIANTS \
  X(NONE, 0, "NONE")     \
  X(SOME, 1, "SOME")     \
  X(QUIT, 2, "QUIT")

typedef enum MessageTag MessageTag;
enum MessageTag {
#define X(variant, i, str) MSG_TAG_##variant = i,
  MSG_TAG_VARIANTS
#undef X
};

typedef struct Message Message;
struct Message {
  MessageTag tag;
  intptr_t value;
};

/// A thread-safe bounded message queue
typedef struct MessageQueue MessageQueue;

///
/// Returns the error message associated with a return code.
///
/// @param rc A return code.
/// @return The error message associated with the given return code, or NULL if the return code is invalid.
///
const char* msgq_error(int rc);

///
/// Return the tag string associated with a message tag.
///
/// @param tag A message tag.
/// @return The tag string associated with the given tag, or NULL if the tag is invalid.
///
const char* msgq_tag(MessageTag tag);

///
/// Creates a new bounded queue with the given capacity.
///
/// Allocates memory for the queue and initializes it.
///
/// @param capacity The maximum number of messages the queue can hold.
/// @return A pointer to a new MessageQueue, or NULL on error.
///
MessageQueue* msgq_create(uint32_t capacity);

///
/// Adds an message to the back of the queue.
///
/// @param queue A MessageQueue.
/// @param in Message to add to the back of the queue.
/// @return 0 if the message was added to the queue, 1 if the queue is full, or a negative value on error.
///
int msgq_put(MessageQueue* queue, Message* in);

///
/// Removes and returns the message at the front of the queue, blocking if the queue is empty.
///
/// @param queue A MessageQueue.
/// @param out The message at the front of the queue.
/// @return 0 if a message was removed from the queue, or a negative value on error.
///
int msgq_get(MessageQueue* queue, Message* out);

///
/// Returns the number of messages in the queue.
///
/// @param queue A MessageQueue.
/// @return The number of messages in the queue.
///
uint32_t msgq_size(MessageQueue* queue);

///
/// Frees resources associated with the queue.
///
/// Also frees the queue itself.
///
/// Consider using _cleanup_msgq_ for scoped cleanup.
///
/// @param queue A MessageQueue.
/// @see msgq_create()
///
void msgq_destroy(MessageQueue* queue);

DEFINE_TRIVIAL_CLEANUP_FUNC(MessageQueue*, msgq_destroy)
#define _cleanup_msgq_ _cleanup_(msgq_destroyp)
