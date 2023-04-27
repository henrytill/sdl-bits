#include <stdio.h>

#include <SDL.h>

#include "queue.h"

static const char *const errormsg[] = {
  [-QUEUE_FAILURE_MALLOC] = "malloc failed",
  [-QUEUE_FAILURE_SEM_CREATE] = "SDL_CreateSemaphore failed",
  [-QUEUE_FAILURE_SEM_POST] = "SDL_SemPost failed",
  [-QUEUE_FAILURE_SEM_TRY_WAIT] = "SDL_SemTryWait failed",
  [-QUEUE_FAILURE_SEM_WAIT] = "SDL_SemWait failed",
  [-QUEUE_FAILURE_MUTEX_CREATE] = "SDL_CreateMutex failed",
  [-QUEUE_FAILURE_MUTEX_LOCK] = "SDL_LockMutex failed",
  [-QUEUE_FAILURE_MUTEX_UNLOCK] = "SDL_UnlockMutex failed",
};

const char *queue_error(int rc) {
  if (rc > QUEUE_FAILURE_MALLOC || rc < QUEUE_FAILURE_MIN) {
    return NULL;
  }
  const char *const err = errormsg[-rc];
  if (err == NULL) {
    return NULL;
  }
  const size_t len = strlen(err);
  char *ret = malloc(len + 1);
  if (ret == NULL) {
    return NULL;
  }
  strcpy(ret, err);
  return ret;
}

int queue_init(struct Queue *q, uint32_t capacity) {
  q->buffer = malloc((size_t)capacity * sizeof(q->buffer));
  if (q->buffer == NULL) {
    return QUEUE_FAILURE_MALLOC;
  }
  q->capacity = capacity;
  q->front = 0;
  q->rear = 0;
  q->empty = SDL_CreateSemaphore(capacity);
  if (q->empty == NULL) {
    return QUEUE_FAILURE_SEM_CREATE;
  }
  q->full = SDL_CreateSemaphore(0);
  if (q->empty == NULL) {
    return QUEUE_FAILURE_SEM_CREATE;
  }
  q->lock = SDL_CreateMutex();
  if (q->lock == NULL) {
    return QUEUE_FAILURE_MUTEX_CREATE;
  }
  return 0;
}

int queue_put(struct Queue *q, void *in) {
  int rc;

  rc = SDL_SemTryWait(q->empty);
  if (rc == SDL_MUTEX_TIMEDOUT) {
    return 1;
  } else if (rc != 0) {
    return QUEUE_FAILURE_SEM_TRY_WAIT;
  }
  rc = SDL_LockMutex(q->lock);
  if (rc != 0) {
    return QUEUE_FAILURE_MUTEX_LOCK;
  }
  q->buffer[q->rear] = in;
  q->rear = (q->rear + 1) % q->capacity;
  rc = SDL_UnlockMutex(q->lock);
  if (rc != 0) {
    return QUEUE_FAILURE_MUTEX_UNLOCK;
  }
  rc = SDL_SemPost(q->full);
  if (rc != 0) {
    return QUEUE_FAILURE_SEM_POST;
  }
  return 0;
}

int queue_get(struct Queue *q, __attribute__((unused)) void **out) {
  int rc;

  rc = SDL_SemWait(q->full);
  if (rc != 0) {
    return QUEUE_FAILURE_SEM_WAIT;
  }
  rc = SDL_LockMutex(q->lock);
  if (rc != 0) {
    return QUEUE_FAILURE_MUTEX_LOCK;
  }
  *out = q->buffer[q->front];
  q->front = (q->front + 1) % q->capacity;
  rc = SDL_UnlockMutex(q->lock);
  if (rc != 0) {
    return QUEUE_FAILURE_MUTEX_UNLOCK;
  }
  rc = SDL_SemPost(q->empty);
  if (rc != 0) {
    return QUEUE_FAILURE_SEM_POST;
  }
  return 0;
}

uint32_t queue_size(struct Queue *q) {
  if (q == NULL) return 0;
  return SDL_SemValue(q->full);
}

void queue_finish(struct Queue *q) {
  if (q == NULL) return;
  if (q->buffer != NULL) free(q->buffer);
  if (q->empty != NULL) SDL_DestroySemaphore(q->empty);
  if (q->full != NULL) SDL_DestroySemaphore(q->full);
  if (q->lock != NULL) SDL_DestroyMutex(q->lock);
}
