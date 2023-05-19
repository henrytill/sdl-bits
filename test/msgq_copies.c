///
/// Test that values are copied into and out of the message queue.
///
/// The producer thread produces messages with values 42, 0, and 1.
/// The consumer consumes messages on the main thread after a delay and checks
/// their values.
///
/// @see msgq_create()
/// @see msgq_put()
/// @see msgq_get()
/// @see msgq_destroy()
///
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL.h>

#include "msgq.h"
#include "prelude.h"

#define LOG(msg) ({                              \
  msgq_message __msg = (msg);                    \
  SDL_LogInfo(APP, "%s: %s{%s, %" PRIdPTR "}",   \
              __func__, #msg,                    \
              msgq_tag(__msg.tag), __msg.value); \
})

#define CHECK(msg, expected_tag, expected_value) ({            \
  msgq_message __msg = (msg);                                  \
  int __tag = (expected_tag);                                  \
  typeof(expected_value) __value = (expected_value);           \
  if (__msg.tag != __tag || __msg.value != __value) {          \
    SDL_LogError(ERR, "%s: %s{%s, %" PRIdPTR "} != {%s, %ld}", \
                 __func__, #msg,                               \
                 msgq_tag(__msg.tag), __msg.value,             \
                 msgq_tag(__tag), __value);                    \
    exit(EXIT_FAILURE);                                        \
  }                                                            \
});

/// Delay before consuming messages.
static const uint32_t DELAY = 2000U;

/// Capacity of the msgq_queue.
static const uint32_t QUEUE_CAP = 1U;

/// Log an error message and exit.
static void fail(const char *msg) {
  SDL_LogError(ERR, "%s", msg);
  exit(EXIT_FAILURE);
}

/// Log a msgq_queue error message and exit.
static void msgq_fail(int rc, const char *msg) {
  SDL_LogError(ERR, "%s: %s", msg, msgq_failure(rc));
  exit(EXIT_FAILURE);
}

/// Log a SDL error message and exit.
static void sdl_fail(const char *msg) {
  sdl_error(msg);
  exit(EXIT_FAILURE);
}

///
/// Produce messages.
///
/// This function is meant to be run in its own thread by passing it to SDL_CreateThread().
///
/// @param data Pointer to a msgq_queue.
/// @return 0 on success, 1 on failure.
/// @see consume()
///
static int produce(void *data) {
  msgq_queue *queue = (msgq_queue *)data;
  msgq_message msg = {.tag = MSG_TAG_SOME, .value = 42};

  for (int rc = 1; rc == 1;) {
    rc = msgq_put(queue, &msg);
    if (rc < 0) {
      msgq_fail(rc, "msgq_put failed");
    }
  }
  LOG(msg);

  msg.tag = MSG_TAG_SOME;
  msg.value = 0;
  for (int rc = 1; rc == 1;) {
    rc = msgq_put(queue, &msg);
    if (rc < 0) {
      msgq_fail(rc, "msgq_put failed");
    }
  }
  LOG(msg);

  msg.tag = MSG_TAG_SOME;
  msg.value = 1;
  for (int rc = 1; rc == 1;) {
    rc = msgq_put(queue, &msg);
    if (rc < 0) {
      msgq_fail(rc, "msgq_put failed");
    }
  }
  LOG(msg);

  return 0;
}

///
/// Consume messages produced by produce() after a delay and check their values.
///
/// This function is meant to be run in the main thread.
///
/// @param queue Pointer to a msgq_queue.
/// @return 0 on success, 1 on failure.
/// @see produce()
///
static int consume(msgq_queue *queue) {
  extern const uint32_t DELAY;

  msgq_message a = {0};
  msgq_message b = {0};
  msgq_message c = {0};

  SDL_LogInfo(APP, "%s: pausing for %d...", __func__, DELAY);
  SDL_Delay(DELAY);

  msgq_get(queue, &a);
  LOG(a);
  CHECK(a, MSG_TAG_SOME, 42L);

  msgq_get(queue, &b);
  LOG(b);
  CHECK(a, MSG_TAG_SOME, 42L);
  CHECK(b, MSG_TAG_SOME, 0L);

  msgq_get(queue, &c);
  LOG(c);
  CHECK(a, MSG_TAG_SOME, 42L);
  CHECK(b, MSG_TAG_SOME, 0L);
  CHECK(c, MSG_TAG_SOME, 1L);

  return 0;
}

///
/// Initialize SDL and a msgq_queue, run the producer thread, Consume, and clean up.
///
int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
  extern const uint32_t QUEUE_CAP;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  int rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc != 0) {
    sdl_fail("SDL_Init failed");
  }

  AT_EXIT(SDL_Quit);

  SCOPED_PTR_msgq_queue queue = msgq_create(QUEUE_CAP);
  if (queue == NULL) {
    fail("msgq_create failed");
  }

  SDL_Thread *producer = SDL_CreateThread(produce, "producer", queue);
  if (producer == NULL) {
    sdl_fail("SDL_CreateThread failed");
  }

  if (consume(queue) != 0) {
    return EXIT_FAILURE;
  }

  SDL_WaitThread(producer, NULL);
  return EXIT_SUCCESS;
}
