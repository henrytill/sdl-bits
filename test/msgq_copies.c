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

#define LOG(msg) ({                            \
  SDL_LogInfo(APP, "%s: %s{%s, %" PRIdPTR "}", \
              __func__, #msg,                  \
              msgq_tag(msg.tag), msg.value);   \
})

#define CHECK(msg, exTag, exVal) ({                            \
  if (msg.tag != exTag || msg.value != exVal) {                \
    SDL_LogError(ERR, "%s: %s{%s, %" PRIdPTR "} != {%s, %ld}", \
                 __func__, #msg,                               \
                 msgq_tag(msg.tag), msg.value,                 \
                 msgq_tag(exTag), exVal);                      \
    exit(EXIT_FAILURE);                                        \
  }                                                            \
});

/// Delay before consuming messages.
static const uint32_t delay = 2000U;

/// Capacity of the MessageQueue.
static const uint32_t queueCap = 1U;

/// Log an error message and exit.
static void fail(const char *msg) {
  SDL_LogError(ERR, "%s", msg);
  exit(EXIT_FAILURE);
}

/// Log a msgq error message and exit.
static void msgq_fail(int err, const char *msg) {
  SDL_LogError(ERR, "%s: %s", msg, msgq_error(err));
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
/// @param data Pointer to a MessageQueue.
/// @return 0 on success, 1 on failure.
/// @see consume()
///
static int produce(void *data) {
  struct MessageQueue *queue = (struct MessageQueue *)data;
  struct Message msg = {.tag = MSG_TAG_SOME, .value = 42};

  for (int rc = 1; rc == 1;) {
    rc = msgq_put(queue, &msg);
    if (rc < 0)
      msgq_fail(rc, "msgq_put failed");
  }
  LOG(msg);

  msg.tag = MSG_TAG_SOME;
  msg.value = 0;
  for (int rc = 1; rc == 1;) {
    rc = msgq_put(queue, &msg);
    if (rc < 0)
      msgq_fail(rc, "msgq_put failed");
  }
  LOG(msg);

  msg.tag = MSG_TAG_SOME;
  msg.value = 1;
  for (int rc = 1; rc == 1;) {
    rc = msgq_put(queue, &msg);
    if (rc < 0)
      msgq_fail(rc, "msgq_put failed");
  }
  LOG(msg);

  return 0;
}

///
/// Consume messages produced by produce() after a delay and check their values.
///
/// This function is meant to be run in the main thread.
///
/// @param queue Pointer to a MessageQueue.
/// @return 0 on success, 1 on failure.
/// @see produce()
///
static int consume(struct MessageQueue *queue) {
  extern const uint32_t delay;

  struct Message a = {0};
  struct Message b = {0};
  struct Message c = {0};

  SDL_LogInfo(APP, "%s: pausing for %d...", __func__, delay);
  SDL_Delay(delay);

  msgq_get(queue, &a);
  LOG(a);
  CHECK(a, MSG_TAG_SOME, 42l);

  msgq_get(queue, &b);
  LOG(b);
  CHECK(a, MSG_TAG_SOME, 42l);
  CHECK(b, MSG_TAG_SOME, 0l);

  msgq_get(queue, &c);
  LOG(c);
  CHECK(a, MSG_TAG_SOME, 42l);
  CHECK(b, MSG_TAG_SOME, 0l);
  CHECK(c, MSG_TAG_SOME, 1l);

  return 0;
}

///
/// Initialize SDL and a MessageQueue, run the producer thread, consume, and clean up.
///
int main(_unused_ int argc, _unused_ char *argv[]) {
  extern const uint32_t queueCap;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  int rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc != 0)
    sdl_fail("SDL_Init failed");

  AT_EXIT(SDL_Quit);

  _cleanup_msgq_ struct MessageQueue *queue = msgq_create(queueCap);
  if (queue == NULL)
    fail("msgq_create failed");

  SDL_Thread *producer = SDL_CreateThread(produce, "producer", queue);
  if (producer == NULL)
    sdl_fail("SDL_CreateThread failed");

  if (consume(queue) != 0)
    return EXIT_FAILURE;

  SDL_WaitThread(producer, NULL);
  return EXIT_SUCCESS;
}
