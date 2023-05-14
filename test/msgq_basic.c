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

#include "msgq.h"
#include "prelude.h"

/// Maximum value to produce.
static const int count = 100;

/// Capacity of the MessageQueue.
static const uint32_t queueCap = 4;

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
/// Produce messages with values from 0 to count. The last message has
/// tag MSG_TAG_QUIT.
///
/// This function is meant to be run in its own thread by passing it to SDL_CreateThread().
///
/// @param data Pointer to a MessageQueue.
/// @return 0 on success
/// @see consume()
///
static int produce(void *data) {
  extern const int count;

  struct Message msg = {0};
  enum MessageTag tag = MSG_TAG_NONE;
  const char *tagStr = NULL;

  if (data == NULL)
    fail("produce failed: data is NULL");

  struct MessageQueue *queue = (struct MessageQueue *)data;

  for (intptr_t value = 0; value <= count;) {
    tag = (value < count) ? MSG_TAG_SOME : MSG_TAG_QUIT;
    tagStr = msgq_tag(tag);

    msg.tag = tag;
    msg.value = value;

    const int rc = msgq_put(queue, &msg);
    if (rc < 0) {
      msgq_fail(rc, "msgq_put failed");
    } else if (rc == 1) {
      SDL_LogDebug(APP, "produce {%s, %" PRIdPTR "} blocked: retrying",
                   tagStr, value);
      continue;
    } else {
      SDL_LogInfo(APP, "Produced {%s, %" PRIdPTR "}",
                  tagStr, value);
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
/// @return 0 when a message with tag MSG_TAG_QUIT is received, 1 when a message with tag MSG_TAG_SOME is received,
/// @see produce()
///
static int consume(struct MessageQueue *queue) {
  struct Message msg;

  const int rc = msgq_get(queue, &msg);
  if (rc < 0)
    msgq_fail(rc, "msgq_get failed");

  SDL_LogInfo(APP, "Consumed {%s, %" PRIdPTR "}",
              msgq_tag(msg.tag), msg.value);

  return msg.tag != MSG_TAG_QUIT;
}

///
/// Initialize SDL and a MessageQueue, run the producer thread, consume, and clean up.
///
int main(_unused_ int argc, _unused_ char *argv[]) {
  extern struct MessageQueue queue;
  extern const uint32_t queueCap;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  int rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc < 0)
    sdl_fail("SDL_Init failed");

  AT_EXIT(SDL_Quit);

  rc = msgq_init(&queue, queueCap);
  if (rc < 0)
    msgq_fail(rc, "msgq_init failed");

  AT_EXIT(finishQueue);

  SDL_Thread *producer = SDL_CreateThread(produce, "producer", &queue);
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
