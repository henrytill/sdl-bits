#include <SDL.h>

#include "msgq.h"

/// A thread-safe message queue
struct MessageQueue {
  struct Message* buffer; // Buffer to hold messages
  uint32_t capacity;      // Maximum size of the buffer
  size_t front;           // Index of the front message in the buffer
  size_t rear;            // Index of the rear message in the buffer
  SDL_sem* empty;         // Semaphore to track empty slots in the buffer
  SDL_sem* full;          // Semaphore to track filled slots in the buffer
  SDL_mutex* lock;        // Mutex lock to protect buffer access
};

static const char* const errorStr[] = {
#define X(variant, i, str) [-(MSGQ_FAILURE_##variant)] = str,
  MSGQ_FAILURE_VARIANTS
#undef X
};

const char* msgq_error(int rc) {
  extern const char* const errorStr[];
  if (rc > MSGQ_FAILURE_MALLOC || rc < MSGQ_FAILURE_MUTEX_UNLOCK) {
    return NULL;
  }
  return errorStr[-rc];
}

static const char* const tagStr[] = {
#define X(variant, i, str) [MSG_TAG_##variant] = str,
  MSG_TAG_VARIANTS
#undef X
};

const char* msgq_tag(enum MessageTag tag) {
  extern const char* const tagStr[];
  if (tag > MSG_TAG_QUIT || tag < MSG_TAG_NONE) {
    return NULL;
  }
  return tagStr[tag];
}

static int msgq_init(struct MessageQueue* queue, uint32_t capacity) {
  queue->buffer = calloc((size_t)capacity, sizeof(*queue->buffer));
  if (queue->buffer == NULL) {
    return MSGQ_FAILURE_MALLOC;
  }
  queue->capacity = capacity;
  queue->front = 0;
  queue->rear = 0;
  queue->empty = SDL_CreateSemaphore(capacity);
  if (queue->empty == NULL) {
    free(queue->buffer);
    return MSGQ_FAILURE_SEM_CREATE;
  }
  queue->full = SDL_CreateSemaphore(0);
  if (queue->full == NULL) {
    SDL_DestroySemaphore(queue->empty);
    free(queue->buffer);
    return MSGQ_FAILURE_SEM_CREATE;
  }
  queue->lock = SDL_CreateMutex();
  if (queue->lock == NULL) {
    SDL_DestroySemaphore(queue->full);
    SDL_DestroySemaphore(queue->empty);
    free(queue->buffer);
    return MSGQ_FAILURE_MUTEX_CREATE;
  }
  return 0;
}

struct MessageQueue* msgq_create(uint32_t capacity) {
  struct MessageQueue* queue = calloc(1, sizeof(*queue));
  if (queue == NULL) {
    return NULL;
  }
  int rc = msgq_init(queue, capacity);
  if (rc < 0) {
    free(queue);
    return NULL;
  }
  return queue;
}

int msgq_put(struct MessageQueue* queue, struct Message* in) {
  int rc = SDL_SemTryWait(queue->empty);
  if (rc == SDL_MUTEX_TIMEDOUT) {
    return 1;
  } else if (rc < 0) {
    return MSGQ_FAILURE_SEM_TRY_WAIT;
  }
  rc = SDL_LockMutex(queue->lock);
  if (rc == -1) {
    return MSGQ_FAILURE_MUTEX_LOCK;
  }
  queue->buffer[queue->rear] = *in;
  queue->rear = (queue->rear + 1) % queue->capacity;
  rc = SDL_UnlockMutex(queue->lock);
  if (rc == -1) {
    return MSGQ_FAILURE_MUTEX_UNLOCK;
  }
  rc = SDL_SemPost(queue->full);
  if (rc < 0) {
    return MSGQ_FAILURE_SEM_POST;
  }
  return 0;
}

int msgq_get(struct MessageQueue* queue, struct Message* out) {
  int rc = SDL_SemWait(queue->full);
  if (rc < 0) {
    return MSGQ_FAILURE_SEM_WAIT;
  }
  rc = SDL_LockMutex(queue->lock);
  if (rc == -1) {
    return MSGQ_FAILURE_MUTEX_LOCK;
  }
  *out = queue->buffer[queue->front];
  queue->front = (queue->front + 1) % queue->capacity;
  rc = SDL_UnlockMutex(queue->lock);
  if (rc == -1) {
    return MSGQ_FAILURE_MUTEX_UNLOCK;
  }
  rc = SDL_SemPost(queue->empty);
  if (rc < 0) {
    return MSGQ_FAILURE_SEM_POST;
  }
  return 0;
}

uint32_t msgq_size(struct MessageQueue* queue) {
  if (queue == NULL) return 0;
  return SDL_SemValue(queue->full);
}

static void msgq_finish(struct MessageQueue* queue) {
  if (queue == NULL) return;
  queue->capacity = 0;
  queue->front = 0;
  queue->rear = 0;
  if (queue->buffer != NULL) {
    free(queue->buffer);
    queue->buffer = NULL;
  }
  if (queue->empty != NULL) {
    SDL_DestroySemaphore(queue->empty);
    queue->empty = NULL;
  }
  if (queue->full != NULL) {
    SDL_DestroySemaphore(queue->full);
    queue->full = NULL;
  }
  if (queue->lock != NULL) {
    SDL_DestroyMutex(queue->lock);
    queue->lock = NULL;
  }
}

void msgq_destroy(struct MessageQueue* queue) {
  if (queue == NULL) return;
  msgq_finish(queue);
  free(queue);
}
