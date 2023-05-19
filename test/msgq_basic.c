///
/// Test basic message queue functionality.
///
/// The producer thread produces messages with values from 0 to COUNT.
/// The consumer consumes messages on the main thread until it receives a
/// message with tag MSG_TAG_QUIT.
///
/// @see msgq_create()
/// @see msgq_put()
/// @see msgq_get()
/// @see msgq_destroy()
///
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "msgq.h"
#include "prelude.h"

/// Maximum value to Produce.
static const int COUNT = 100;

/// Capacity of the msgq_queue.
static const uint32_t QUEUE_CAP = 4U;

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
/// Produce messages with values from 0 to COUNT. The last message has tag MSG_TAG_QUIT.
///
/// This function is meant to be run in its own thread by passing it to SDL_CreateThread().
///
/// @param data Pointer to a msgq_queue.
/// @return 0 on success
/// @see consume()
///
static int produce(void *data) {
  extern const int COUNT;

  if (data == NULL) {
    fail("produce failed: data is NULL");
  }

  msgq_queue *queue = (msgq_queue *)data;
  msgq_message msg = {0};
  const char *tag_str = NULL;
  int rc = 0;

  for (intptr_t value = 0; value <= COUNT;) {
    msg.tag = (value < COUNT) ? MSG_TAG_SOME : MSG_TAG_QUIT;
    msg.value = value;
    tag_str = msgq_tag(msg.tag);

    rc = msgq_put(queue, &msg);
    if (rc < 0) {
      msgq_fail(rc, "msgq_put failed");
    } else if (rc == 1) {
      SDL_LogDebug(APP, "produce {%s, %" PRIdPTR "} blocked: retrying",
                   tag_str, value);
      continue;
    } else {
      SDL_LogInfo(APP, "Produced {%s, %" PRIdPTR "}",
                  tag_str, value);
      value += 1;
    }
  }

  return 0;
}

///
/// Consume messages until a message with tag MSG_TAG_QUIT is received.
///
/// This function is meant to be run on the main thread.
///
/// @param queue Pointer to a msgq_queue.
/// @param out Pointer to a message.
/// @return 0 when a message with tag MSG_TAG_QUIT is received, 1 otherwise
/// @see produce()
///
static int consume(msgq_queue *queue, msgq_message *out) {
  const int rc = msgq_get(queue, out);
  if (rc < 0) {
    msgq_fail(rc, "msgq_get failed");
  }
  SDL_LogInfo(APP, "Consumed {%s, %" PRIdPTR "}",
              msgq_tag(out->tag), out->value);
  return out->tag != MSG_TAG_QUIT;
}

///
/// Initialize SDL and a msgq_queue, run the producer thread, Consume, and clean up.
///
int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
  extern const uint32_t QUEUE_CAP;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  int rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc < 0) {
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

  msgq_message msg;
  for (;;) {
    rc = consume(queue, &msg);
    if (rc == 0) {
      break;
    }
    if (rc < 0) {
      return EXIT_FAILURE;
    }
  }

  SDL_WaitThread(producer, NULL);
  return EXIT_SUCCESS;
}
