#ifndef SDL_BITS_QUEUE_H
#define SDL_BITS_QUEUE_H

#include <stdlib.h>

enum {
  QUEUE_FAILURE_MALLOC = -1,
  QUEUE_FAILURE_SEM_CREATE = -2,
  QUEUE_FAILURE_SEM_POST = -3,
  QUEUE_FAILURE_SEM_TRY_WAIT = -4,
  QUEUE_FAILURE_SEM_WAIT = -5,
  QUEUE_FAILURE_MUTEX_CREATE = -6,
  QUEUE_FAILURE_MUTEX_LOCK = -7,
  QUEUE_FAILURE_MUTEX_UNLOCK = -8,
  QUEUE_FAILURE_MIN = -9,
};

/* A synchronous bounded queue */
struct Queue {
  void **buffer;     // circular buffer to hold data
  uint32_t capacity; // maximum size of the buffer
  size_t front;      // index of the front element in the buffer
  size_t rear;       // index of the rear element in the buffer
  SDL_sem *empty;    // semaphore to track empty slots in the buffer
  SDL_sem *full;     // semaphore to track filled slots in the buffer
  SDL_mutex *lock;   // mutex lock to protect buffer access
};

/*
 * Returns the error message associated with a return code.
 *
 * Returns NULL if the return code has no associated error message.
 */
const char *queue_error(int rc);

/*
 * Initializes a new bounded queue with the given capacity.
 *
 * Returns 0 on success, or a negative value on error.
 */
int queue_init(struct Queue *q, uint32_t capacity);

/*
 * Adds an element to the back of the queue.
 *
 * Returns 0 if the element was added to the queue, 1 if the queue is full, or a negative value on error.
 */
int queue_put(struct Queue *q, void *in);

/*
 * Removes and returns the element at the front of the queue, blocking if the queue is empty.
 *
 * Returns 0 if the element was removed from the queue, or a negative value on error.
 */
int queue_get(struct Queue *q, void **out);

/*
 * Returns the number of elements in the queue.
 */
uint32_t queue_size(struct Queue *q);

/* Frees resources associated with the queue. */
void queue_finish(struct Queue *q);

#endif /* SDL_BITS_QUEUE_H */
