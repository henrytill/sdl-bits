#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

///
/// Create a semaphore.
///
/// @param value The initial value of the semaphore.
/// @return The semaphore, or NULL on failure.
///
static inline sem_t *createSemaphore(uint32_t value) {
  sem_t *sem = calloc(1, sizeof(sem_t));
  if (sem == NULL) {
    return NULL;
  }
  const int rc = sem_init(sem, 0, value);
  if (rc == -1) {
    free(sem);
    return NULL;
  }
  return sem;
}

///
/// Create a mutex.
///
/// @return The mutex, or NULL on failure.
///
static inline pthread_mutex_t *createMutex(void) {
  pthread_mutex_t *mutex = calloc(1, sizeof(pthread_mutex_t));
  if (mutex == NULL) {
    return NULL;
  }
  const int rc = pthread_mutex_init(mutex, NULL);
  if (rc != 0) {
    free(mutex);
    return NULL;
  }
  return mutex;
}

///
/// Destroy a semaphore.
///
/// @param sem The semaphore to destroy.
///
static inline void destroySemaphore(sem_t *sem) {
  if (sem == NULL) {
    return;
  }
  sem_destroy(sem);
  free(sem);
}

///
/// Destroy a mutex.
///
/// @param mutex The mutex to destroy.
///
static inline void destroyMutex(pthread_mutex_t *mutex) {
  if (mutex == NULL) {
    return;
  }
  pthread_mutex_destroy(mutex);
  free(mutex);
}
