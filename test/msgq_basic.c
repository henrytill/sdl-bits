///
/// Test basic message queue functionality.
///
/// The producer thread produces messages with values from 0 to count.
/// The consumer consumes messages on the main thread until it receives a
/// message with tag NONE.
///
/// @see msgq_init()
/// @see msgq_put()
/// @see msgq_get()
/// @see msgq_destroy()
///
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL.h>

#include "macro.h"
#include "msgq.h"

/// Log categories to use with SDL logging functions.
enum LogCategory {
  APP = SDL_LOG_CATEGORY_CUSTOM,
  ERR,
};

/// Maximum value to produce.
static const int count = 100;

/// Capacity of the MessageQueue.
static const uint32_t queueCapacity = 4;

/// MessageQueue for testing.
static struct MessageQueue queue;

///
/// Call msgq_finish() on queue.
///
/// @see msgq_finish()
///
static void finishQueue(void) {
  extern struct MessageQueue queue;
  msgq_finish(&queue);
}

/// Log an error message and exits.
static void fail(const char *msg) {
  SDL_LogError(ERR, "%s", msg);
  exit(EXIT_FAILURE);
}

/// Log a msgq error message and exits.
static void msgq_fail(int err, const char *msg) {
  SDL_LogError(ERR, "%s: %s", msg, msgq_error(err));
  exit(EXIT_FAILURE);
}

/// Log a SDL error message and exits.
static void sdl_fail(const char *msg) {
  const char *err = SDL_GetError();
  if (strlen(err) != 0)
    SDL_LogError(ERR, "%s (%s)", msg, err);
  else
    SDL_LogError(ERR, "%s", msg);
  exit(EXIT_FAILURE);
}

///
/// Produce messages with values from 0 to count. The last message has
/// tag NONE.
///
/// This function is meant to be run in its own thread by passing it to SDL_CreateThread().
///
/// @param data Pointer to a MessageQueue.
/// @return 0 on success
/// @see consume()
///
static int produce(void *data) {
  extern const int count;

  int rc;
  struct Message message;
  enum MessageTag tag;
  const char *tagString = NULL;

  if (data == NULL)
    fail("produce failed: data is NULL");

  struct MessageQueue *queue = (struct MessageQueue *)data;

  for (intptr_t value = 0; value <= count;) {
    tag = (value < count) ? SOME : NONE;
    tagString = msgq_tag(tag);

    message.tag = tag;
    message.value = value;

    rc = msgq_put(queue, (void *)&message);
    if (rc < 0) {
      msgq_fail(rc, "msgq_put failed");
    } else if (rc == 1) {
      SDL_LogDebug(APP, "produce {%s, %" PRIdPTR "} blocked: retrying",
                   tagString, value);
      continue;
    } else {
      SDL_LogInfo(APP, "Produced {%s, %" PRIdPTR "}",
                  tagString, value);
      value += 1;
    }
  }

  return 0;
}

///
/// Consume messages until a message with tag NONE is received.
///
/// This function is meant to be run on the main thread.
///
/// @param queue Pointer to a MessageQueue.
/// @return 0 when a message with tag NONE is received, 1 when a message with tag SOME is received,
/// @see produce()
///
static int consume(struct MessageQueue *queue) {
  struct Message message;

  const int rc = msgq_get(queue, (void *)&message);
  if (rc < 0)
    msgq_fail(rc, "msgq_get failed");

  SDL_LogInfo(APP, "Consumed {%s, %" PRIdPTR "}",
              msgq_tag(message.tag), message.value);

  return message.tag != NONE;
}

///
/// Initialize SDL and a MessageQueue, run the producer thread, consume, and clean up.
///
int main(_unused_ int argc, _unused_ char *argv[]) {
  extern struct MessageQueue queue;
  extern const uint32_t queueCapacity;

  int rc;
  SDL_Thread *producer;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc < 0)
    sdl_fail("SDL_Init failed");

  AT_EXIT(SDL_Quit);

  rc = msgq_init(&queue, queueCapacity);
  if (rc < 0)
    msgq_fail(rc, "msgq_init failed");

  AT_EXIT(finishQueue);

  producer = SDL_CreateThread(produce, "producer", &queue);
  if (producer == NULL)
    sdl_fail("SDL_CreateThread failed");

  for (;;) {
    rc = consume(&queue);
    if (rc == 0) {
      break;
    } else if (rc < 0) {
      return EXIT_FAILURE;
    }
  }

  SDL_WaitThread(producer, NULL);
  return EXIT_SUCCESS;
}
