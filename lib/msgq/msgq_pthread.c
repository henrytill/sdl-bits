#include <errno.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

#include "msgq.h"

struct MessageQueue {
  Message* buffer;       // Buffer to hold messages
  uint32_t capacity;     // Maximum size of the buffer
  size_t front;          // Index of the front message in the buffer
  size_t rear;           // Index of the rear message in the buffer
  sem_t* empty;          // Semaphore to track empty slots in the buffer
  sem_t* full;           // Semaphore to track filled slots in the buffer
  pthread_mutex_t* lock; // Mutex lock to protect buffer access
};

static const char* const messageQueueFailureStr[] = {
#define X(variant, i, str) [-(MSGQ_FAILURE_##variant)] = str,
  MSGQ_FAILURE_VARIANTS
#undef X
};

static const char* const messageTagStr[] = {
#define X(variant, i, str) [MSG_TAG_##variant] = str,
  MSG_TAG_VARIANTS
#undef X
};

const char* msgq_Failure(int rc) {
  extern const char* const messageQueueFailureStr[];
  if (rc > MSGQ_FAILURE_MALLOC || rc < MSGQ_FAILURE_MUTEX_UNLOCK) {
    return NULL;
  }
  return messageQueueFailureStr[-rc];
}

const char* msgq_MessageTag(MessageTag tag) {
  extern const char* const messageTagStr[];
  if (tag > MSG_TAG_QUIT || tag < MSG_TAG_NONE) {
    return NULL;
  }
  return messageTagStr[tag];
}

static int msgq_Init(MessageQueue* queue, uint32_t capacity) {
  queue->buffer = calloc((size_t)capacity, sizeof(*queue->buffer));
  if (queue->buffer == NULL) {
    return MSGQ_FAILURE_MALLOC;
  }
  queue->empty = calloc(1, sizeof(*queue->empty));
  if (queue->empty == NULL) {
    free(queue->buffer);
    return MSGQ_FAILURE_MALLOC;
  }
  queue->full = calloc(1, sizeof(*queue->full));
  if (queue->full == NULL) {
    free(queue->empty);
    free(queue->buffer);
    return MSGQ_FAILURE_MALLOC;
  }
  queue->lock = calloc(1, sizeof(*queue->lock));
  if (queue->lock == NULL) {
    free(queue->full);
    free(queue->empty);
    free(queue->buffer);
    return MSGQ_FAILURE_MALLOC;
  }
  queue->capacity = capacity;
  queue->front = 0;
  queue->rear = 0;
  int rc = sem_init(queue->empty, 0, capacity);
  if (rc == -1) {
    free(queue->buffer);
    return MSGQ_FAILURE_SEM_CREATE;
  }
  rc = sem_init(queue->full, 0, 0);
  if (rc == -1) {
    sem_destroy(queue->empty);
    free(queue->buffer);
    return MSGQ_FAILURE_SEM_CREATE;
  }
  rc = pthread_mutex_init(queue->lock, NULL);
  if (rc != 0) {
    sem_destroy(queue->full);
    sem_destroy(queue->empty);
    free(queue->buffer);
    return MSGQ_FAILURE_MUTEX_CREATE;
  }
  return 0;
}

static void msgq_Finish(MessageQueue* queue) {
  if (queue == NULL) return;
  queue->capacity = 0;
  queue->front = 0;
  queue->rear = 0;
  if (queue->buffer != NULL) {
    free(queue->buffer);
    queue->buffer = NULL;
  }
  if (queue->empty != NULL) {
    sem_destroy(queue->empty);
    free(queue->empty);
    queue->empty = NULL;
  }
  if (queue->full != NULL) {
    sem_destroy(queue->full);
    free(queue->full);
    queue->full = NULL;
  }
  if (queue->lock != NULL) {
    pthread_mutex_destroy(queue->lock);
    free(queue->lock);
    queue->lock = NULL;
  }
}

MessageQueue* msgq_Create(uint32_t capacity) {
  MessageQueue* queue = calloc(1, sizeof(*queue));
  if (queue == NULL) {
    return NULL;
  }
  int rc = msgq_Init(queue, capacity);
  if (rc < 0) {
    free(queue);
    return NULL;
  }
  return queue;
}

void msgq_Destroy(MessageQueue* queue) {
  if (queue == NULL) return;
  msgq_Finish(queue);
  free(queue);
}

int msgq_Put(MessageQueue* queue, Message* in) {
  int rc = sem_trywait(queue->empty);
  if (rc == -1 && errno == EAGAIN) {
    return 1;
  } else if (rc == -1) {
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

int msgq_Get(MessageQueue* queue, Message* out) {
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

uint32_t msgq_Size(MessageQueue* queue) {
  if (queue == NULL) return 0;
  int ret = 0;
  sem_getvalue(queue->full, &ret);
  return (uint32_t)ret;
}
