#ifndef SDL_BITS_MSGQ_H
#define SDL_BITS_MSGQ_H

#include <stdint.h>

enum {
  MSGQ_FAILURE_MALLOC = -1,
  MSGQ_FAILURE_SEM_CREATE = -2,
  MSGQ_FAILURE_SEM_POST = -3,
  MSGQ_FAILURE_SEM_TRY_WAIT = -4,
  MSGQ_FAILURE_SEM_WAIT = -5,
  MSGQ_FAILURE_MUTEX_CREATE = -6,
  MSGQ_FAILURE_MUTEX_LOCK = -7,
  MSGQ_FAILURE_MUTEX_UNLOCK = -8,
};

enum MessageTag {
  NONE = 1 << 0,
  SOME = 1 << 1,
  QUIT = 1 << 2,
};

struct Message {
  enum MessageTag tag;
  intptr_t value;
};

/** A synchronous bounded message queue */
struct MessageQueue {
  struct Message *buffer; /* buffer to hold messages */
  uint32_t capacity;      /* maximum size of the buffer */
  size_t front;           /* index of the front message in the buffer */
  size_t rear;            /* index of the rear message in the buffer */
  SDL_sem *empty;         /* semaphore to track empty slots in the buffer */
  SDL_sem *full;          /* semaphore to track filled slots in the buffer */
  SDL_mutex *lock;        /* mutex lock to protect buffer access */
};

/**
 * Return the tag string associated with a message tag.
 *
 * @param tag A message tag.
 * @return The tag string associated with the given tag, or NULL if the tag is invalid.
 */
const char *msgq_tagstr(enum MessageTag tag);

/**
 * Returns the error message associated with a return code.
 *
 * @param rc A return code.
 * @return The error message associated with the given return code, or NULL if the return code is invalid.
 */
const char *msgq_errorstr(int rc);

/**
 * Initializes a new bounded queue with the given capacity.
 *
 * @param q A MessageQueue.
 * @param capacity The maximum number of messages the queue can hold.
 * @return 0 on success, or a negative value on error.
 */
int msgq_init(struct MessageQueue *q, uint32_t capacity);

/**
 * Adds an message to the back of the queue.
 *
 * @param q A MessageQueue.
 * @param in Message to add to the back of the queue.
 * @returns 0 if the message was added to the queue, 1 if the queue is full, or a negative value on error.
 */
int msgq_put(struct MessageQueue *q, struct Message *in);

/**
 * Removes and returns the message at the front of the queue, blocking if the queue is empty.
 *
 * @param q A MessageQueue.
 * @param out The message at the front of the queue.
 * @returns 0 if a message was removed from the queue, or a negative value on error.
 */
int msgq_get(struct MessageQueue *q, struct Message *out);

/**
 * Returns the number of messages in the queue.
 *
 * @param q A MessageQueue.
 * @return The number of messages in the queue.
 */
uint32_t msgq_size(struct MessageQueue *q);

/**
 * Frees resources associated with the queue.
 *
 * @param q A MessageQueue.
 */
void msgq_finish(struct MessageQueue *q);

#endif /* SDL_BITS_MSGQ_H */
