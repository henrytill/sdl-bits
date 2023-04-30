#include <SDL.h>

#include "msgq.h"

static const char *const tagstr[] = {
  [NONE >> 1] = "NONE",
  [SOME >> 1] = "SOME",
  [QUIT >> 1] = "QUIT",
};

const char *msgq_tagstr(int tag) {
  extern const char *const tagstr[];
  if (tag > QUIT || tag < NONE) {
    return NULL;
  }
  return tagstr[tag >> 1];
}

static const char *const errorstr[] = {
  [-MSGQ_FAILURE_MALLOC] = "malloc failed",
  [-MSGQ_FAILURE_SEM_CREATE] = "SDL_CreateSemaphore failed",
  [-MSGQ_FAILURE_SEM_POST] = "SDL_SemPost failed",
  [-MSGQ_FAILURE_SEM_TRY_WAIT] = "SDL_SemTryWait failed",
  [-MSGQ_FAILURE_SEM_WAIT] = "SDL_SemWait failed",
  [-MSGQ_FAILURE_MUTEX_CREATE] = "SDL_CreateMutex failed",
  [-MSGQ_FAILURE_MUTEX_LOCK] = "SDL_LockMutex failed",
  [-MSGQ_FAILURE_MUTEX_UNLOCK] = "SDL_UnlockMutex failed",
};

const char *msgq_errorstr(int rc) {
  extern const char *const errorstr[];
  if (rc > MSGQ_FAILURE_MALLOC || rc < MSGQ_FAILURE_MUTEX_UNLOCK) {
    return NULL;
  }
  return errorstr[-rc];
}

int msgq_init(struct MessageQueue *q, uint32_t capacity) {
  q->buffer = malloc((size_t)capacity * sizeof(*q->buffer));
  if (q->buffer == NULL) {
    return MSGQ_FAILURE_MALLOC;
  }
  q->capacity = capacity;
  q->front = 0;
  q->rear = 0;
  q->empty = SDL_CreateSemaphore(capacity);
  if (q->empty == NULL) {
    return MSGQ_FAILURE_SEM_CREATE;
  }
  q->full = SDL_CreateSemaphore(0);
  if (q->empty == NULL) {
    return MSGQ_FAILURE_SEM_CREATE;
  }
  q->lock = SDL_CreateMutex();
  if (q->lock == NULL) {
    return MSGQ_FAILURE_MUTEX_CREATE;
  }
  return 0;
}

int msgq_put(struct MessageQueue *q, struct Message *in) {
  int rc;

  rc = SDL_SemTryWait(q->empty);
  if (rc == SDL_MUTEX_TIMEDOUT) {
    return 1;
  } else if (rc < 0) {
    return MSGQ_FAILURE_SEM_TRY_WAIT;
  }
  rc = SDL_LockMutex(q->lock);
  if (rc == -1) {
    return MSGQ_FAILURE_MUTEX_LOCK;
  }
  q->buffer[q->rear] = *in;
  q->rear = (q->rear + 1) % q->capacity;
  rc = SDL_UnlockMutex(q->lock);
  if (rc == -1) {
    return MSGQ_FAILURE_MUTEX_UNLOCK;
  }
  rc = SDL_SemPost(q->full);
  if (rc < 0) {
    return MSGQ_FAILURE_SEM_POST;
  }
  return 0;
}

int msgq_get(struct MessageQueue *q, struct Message *out) {
  int rc;

  rc = SDL_SemWait(q->full);
  if (rc < 0) {
    return MSGQ_FAILURE_SEM_WAIT;
  }
  rc = SDL_LockMutex(q->lock);
  if (rc == -1) {
    return MSGQ_FAILURE_MUTEX_LOCK;
  }
  *out = q->buffer[q->front];
  q->front = (q->front + 1) % q->capacity;
  rc = SDL_UnlockMutex(q->lock);
  if (rc == -1) {
    return MSGQ_FAILURE_MUTEX_UNLOCK;
  }
  rc = SDL_SemPost(q->empty);
  if (rc < 0) {
    return MSGQ_FAILURE_SEM_POST;
  }
  return 0;
}

uint32_t msgq_size(struct MessageQueue *q) {
  if (q == NULL) return 0;
  return SDL_SemValue(q->full);
}

void msgq_finish(struct MessageQueue *q) {
  if (q == NULL) return;
  if (q->buffer != NULL) free(q->buffer);
  if (q->empty != NULL) SDL_DestroySemaphore(q->empty);
  if (q->full != NULL) SDL_DestroySemaphore(q->full);
  if (q->lock != NULL) SDL_DestroyMutex(q->lock);
}
