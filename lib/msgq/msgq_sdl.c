#include <SDL.h>

#include "msgq.h"

struct msgq_queue {
  msgq_message *buffer;   // Buffer to hold messages
  uint32_t capacity; // Maximum size of the buffer
  size_t front;      // Index of the front message in the buffer
  size_t rear;       // Index of the rear message in the buffer
  SDL_sem *empty;    // Semaphore to track empty slots in the buffer
  SDL_sem *full;     // Semaphore to track filled slots in the buffer
  SDL_mutex *lock;   // Mutex lock to protect buffer access
};

static const char *const MESSAGE_QUEUE_FAILURE_STR[] = {
#define X(variant, i, str) [-(MSGQ_FAILURE_##variant)] = (str),
  MSGQ_FAILURE_VARIANTS
#undef X
};

static const char *const MESSAGE_TAG_STR[] = {
#define X(variant, i, str) [MSG_TAG_##variant] = (str),
  MSG_TAG_VARIANTS
#undef X
};

const char *msgq_failure(int rc) {
  extern const char *const MESSAGE_QUEUE_FAILURE_STR[];
  if (rc > MSGQ_FAILURE_MALLOC || rc < MSGQ_FAILURE_MUTEX_UNLOCK) {
    return NULL;
  }
  return MESSAGE_QUEUE_FAILURE_STR[-rc];
}

const char *msgq_tag(int tag) {
  extern const char *const MESSAGE_TAG_STR[];
  if (tag > MSG_TAG_QUIT || tag < MSG_TAG_NONE) {
    return NULL;
  }
  return MESSAGE_TAG_STR[tag];
}

static int msgq_init(msgq_queue *queue, uint32_t capacity) {
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

static void msgq_finish(msgq_queue *queue) {
  if (queue == NULL) {
    return;
  }
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

msgq_queue *msgq_create(uint32_t capacity) {
  msgq_queue *queue = calloc(1, sizeof(*queue));
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

void msgq_destroy(msgq_queue *queue) {
  if (queue == NULL) {
    return;
  }
  msgq_finish(queue);
  free(queue);
}

int msgq_put(msgq_queue *queue, msgq_message *in) {
  int rc = SDL_SemTryWait(queue->empty);
  if (rc == SDL_MUTEX_TIMEDOUT) {
    return 1;
  }
  if (rc < 0) {
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

int msgq_get(msgq_queue *queue, msgq_message *out) {
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

uint32_t msgq_size(msgq_queue *queue) {
  if (queue == NULL) {
    return 0;
  }
  return SDL_SemValue(queue->full);
}
