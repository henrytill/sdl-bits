#include <errno.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

#include "compat.h"
#include "message_queue.h"

struct message_queue {
  message *buffer;       // Buffer to hold messages
  uint32_t capacity;     // Maximum size of the buffer
  size_t front;          // Index of the front message in the buffer
  size_t rear;           // Index of the rear message in the buffer
  sem_t *empty;          // Semaphore to track empty slots in the buffer
  sem_t *full;           // Semaphore to track filled slots in the buffer
  pthread_mutex_t *lock; // Mutex lock to protect buffer access
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

static int message_queue_init(message_queue *queue, uint32_t capacity) {
  queue->buffer = calloc((size_t)capacity, sizeof(*queue->buffer));
  if (queue->buffer == NULL) {
    return MSGQ_FAILURE_MALLOC;
  }
  queue->capacity = capacity;
  queue->front = 0;
  queue->rear = 0;
  queue->empty = create_semaphore(capacity);
  if (queue->empty == NULL) {
    free(queue->buffer);
    return MSGQ_FAILURE_SEM_CREATE;
  }
  queue->full = create_semaphore(0);
  if (queue->full == NULL) {
    destroy_semaphore(queue->empty);
    free(queue->buffer);
    return MSGQ_FAILURE_SEM_CREATE;
  };
  queue->lock = create_mutex();
  if (queue->lock == NULL) {
    destroy_semaphore(queue->full);
    destroy_semaphore(queue->empty);
    free(queue->buffer);
    return MSGQ_FAILURE_MUTEX_CREATE;
  }
  return 0;
}

static void message_queue_finish(message_queue *queue) {
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
    destroy_semaphore(queue->empty);
    queue->empty = NULL;
  }
  if (queue->full != NULL) {
    destroy_semaphore(queue->full);
    queue->full = NULL;
  }
  if (queue->lock != NULL) {
    destroy_mutex(queue->lock);
    queue->lock = NULL;
  }
}

message_queue *message_queue_create(uint32_t capacity) {
  message_queue *queue = calloc(1, sizeof(*queue));
  if (queue == NULL) {
    return NULL;
  }
  int rc = message_queue_init(queue, capacity);
  if (rc < 0) {
    free(queue);
    return NULL;
  }
  return queue;
}

void message_queue_destroy(message_queue *queue) {
  if (queue == NULL) {
    return;
  }
  message_queue_finish(queue);
  free(queue);
}

int message_queue_put(message_queue *queue, message *in) {
  int rc = sem_trywait(queue->empty);
  if (rc == -1 && errno == EAGAIN) {
    return 1;
  }
  if (rc == -1) {
    return MSGQ_FAILURE_SEM_TRY_WAIT;
  }
  rc = pthread_mutex_lock(queue->lock);
  if (rc != 0) {
    return MSGQ_FAILURE_MUTEX_LOCK;
  }
  queue->buffer[queue->rear] = *in;
  queue->rear = (queue->rear + 1) % queue->capacity;
  rc = pthread_mutex_unlock(queue->lock);
  if (rc != 0) {
    return MSGQ_FAILURE_MUTEX_UNLOCK;
  }
  rc = sem_post(queue->full);
  if (rc == -1) {
    return MSGQ_FAILURE_SEM_POST;
  }
  return 0;
}

int message_queue_get(message_queue *queue, message *out) {
  int rc = sem_wait(queue->full);
  if (rc == -1) {
    return MSGQ_FAILURE_SEM_WAIT;
  }
  rc = pthread_mutex_lock(queue->lock);
  if (rc != 0) {
    return MSGQ_FAILURE_MUTEX_LOCK;
  }
  *out = queue->buffer[queue->front];
  queue->front = (queue->front + 1) % queue->capacity;
  rc = pthread_mutex_unlock(queue->lock);
  if (rc != 0) {
    return MSGQ_FAILURE_MUTEX_UNLOCK;
  }
  rc = sem_post(queue->empty);
  if (rc == -1) {
    return MSGQ_FAILURE_SEM_POST;
  }
  return 0;
}

uint32_t message_queue_size(message_queue *queue) {
  if (queue == NULL) {
    return 0;
  }
  int ret = 0;
  sem_getvalue(queue->full, &ret);
  return (uint32_t)ret;
}
