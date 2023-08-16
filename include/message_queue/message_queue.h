#pragma once

#include <stdint.h>

#include "macro.h"

#define MSG_TAG_VARIANTS \
  X(NONE, 0, "NONE")     \
  X(SOME, 1, "SOME")     \
  X(QUIT, 2, "QUIT")

enum {
#define X(variant, i, str) MSG_TAG_##variant = (i),
  MSG_TAG_VARIANTS
#undef X
};

struct message {
  int tag;
  intptr_t value;
};

/// A thread-safe bounded message queue
struct message_queue;

///
/// Returns the error message associated with a return code.
///
/// @param rc A return code.
/// @return The error message associated with the given return code, or NULL if the return code is invalid.
///
const char *message_queue_failure(int rc);

///
/// Return the tag string associated with a message tag.
///
/// @param tag A message tag.
/// @return The tag string associated with the given tag, or NULL if the tag is invalid.
///
const char *message_queue_tag(int tag);

///
/// Creates a new bounded queue with the given capacity.
///
/// Allocates memory for the queue and initializes it.
///
/// @param capacity The maximum number of messages the queue can hold.
/// @return A pointer to a new message_queue, or NULL on error.
/// @see message_queue_destroy()
///
struct message_queue *message_queue_create(uint32_t capacity);

///
/// Frees resources associated with the queue.
///
/// Also frees the queue itself.
///
/// @param queue Message queue.
/// @see message_queue_create()
///
void message_queue_destroy(struct message_queue *queue);

///
/// Adds an message to the back of the queue.
///
/// @param queue Message queue.
/// @param in The message to add to the back of the queue.
/// @return 0 if the message was added to the queue, 1 if the queue is full, or a negative value on error.
///
int message_queue_put(struct message_queue *queue, struct message *in);

///
/// Removes and returns the message at the front of the queue, blocking if the queue is empty.
///
/// @param queue Message queue.
/// @param out The message at the front of the queue.
/// @return 0 if a message was removed from the queue, or a negative value on error.
///
int message_queue_get(struct message_queue *queue, struct message *out);

///
/// Returns the number of messages in the queue.
///
/// @param queue Message queue.
/// @return The number of messages in the queue.
///
uint32_t message_queue_size(struct message_queue *queue);
