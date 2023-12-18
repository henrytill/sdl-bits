///
/// Test basic message queue functionality.
///
/// The producer thread produces messages with values from 0 to COUNT.
/// The consumer consumes messages on the main thread until it receives a
/// message with tag MSG_TAG_QUIT.
///
/// @see message_queue_create()
/// @see message_queue_put()
/// @see message_queue_get()
/// @see message_queue_destroy()
///
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "macro.h"
#include "message_queue.h"
#include "prelude_sdl.h"

/// Maximum value to Produce.
static const int COUNT = 100;

/// Capacity of the message_queue.
static const uint32_t QUEUE_CAP = 4U;

/// Log an error message and exit.
static void fail(const char *msg) {
  SDL_LogError(ERR, "%s", msg);
  exit(EXIT_FAILURE);
}

/// Log a message_queue error message and exit.
static void message_queue_fail(int rc, const char *msg) {
  SDL_LogError(ERR, "%s: %s", msg, message_queue_failure(rc));
  exit(EXIT_FAILURE);
}

/// Log a SDL error message and exit.
static void sdl_fail(const char *msg) {
  log_sdl_error(msg);
  exit(EXIT_FAILURE);
}

///
/// Produce messages with values from 0 to COUNT. The last message has tag MSG_TAG_QUIT.
///
/// This function is meant to be run in its own thread by passing it to SDL_CreateThread().
///
/// @param data Pointer to a message_queue.
/// @return 0 on success
/// @see consume()
///
static int produce(void *data) {
  extern const int COUNT;

  if (data == NULL) {
    fail("produce failed: data is NULL");
  }

  struct message_queue *queue = data;
  struct message msg = {0};
  const char *tag_str = NULL;
  int rc = -1;

  for (intptr_t value = 0; value <= COUNT;) {
    msg.tag = (value < COUNT) ? MSG_TAG_SOME : MSG_TAG_QUIT;
    msg.value = value;
    tag_str = message_queue_tag(msg.tag);

    rc = message_queue_put(queue, &msg);
    if (rc < 0) {
      message_queue_fail(rc, "message_queue_put failed");
    } else if (rc == 1) {
      SDL_LogDebug(APP, "produce {%s, %" PRIdPTR "} blocked: retrying",
                   tag_str, value);
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
/// @param queue Pointer to a message_queue.
/// @param out Pointer to a message.
/// @return 0 when a message with tag MSG_TAG_QUIT is received, 1 otherwise
/// @see produce()
///
static int consume(struct message_queue *queue, struct message *out) {
  const int rc = message_queue_get(queue, out);
  if (rc < 0) {
    message_queue_fail(rc, "message_queue_get failed");
  }
  SDL_LogInfo(APP, "Consumed {%s, %" PRIdPTR "}",
              message_queue_tag(out->tag), out->value);
  return out->tag != MSG_TAG_QUIT;
}

///
/// Initialize SDL and a message_queue, run the producer thread, consume,
/// and clean up.
///
int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
  extern const uint32_t QUEUE_CAP;

  int ret = EXIT_FAILURE;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  int rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc < 0) {
    sdl_fail("SDL_Init failed");
  }

  AT_EXIT(SDL_Quit);

  struct message_queue *queue = message_queue_create(QUEUE_CAP);
  if (queue == NULL) {
    fail("message_queue_create failed");
  }

  SDL_Thread *producer = SDL_CreateThread(produce, "producer", queue);
  if (producer == NULL) {
    log_sdl_error("SDL_CreateThread failed");
    goto out_destroy_queue;
  }

  struct message msg;
  for (;;) {
    rc = consume(queue, &msg);
    if (rc == 0) {
      break;
    }
    if (rc < 0) {
      goto out_destroy_queue;
    }
  }

  SDL_WaitThread(producer, NULL);

  ret = EXIT_SUCCESS;
out_destroy_queue:
  message_queue_destroy(queue);
  return ret;
}
