///
/// Test basic message queue functionality.
///
/// The producer thread produces messages with values from 0 to count.
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
static const int count = 100;

/// Capacity of the MessageQueue.
static const uint32_t queueCap = 4U;

/// Log an error message and exit.
static void fail(const char *msg) {
  SDL_LogError(ERR, "%s", msg);
  exit(EXIT_FAILURE);
}

/// Log a msgq error message and exit.
static void msgq_fail(int rc, const char *msg) {
  SDL_LogError(ERR, "%s: %s", msg, msgq_failureStr(rc));
  exit(EXIT_FAILURE);
}

/// Log a SDL error message and exit.
static void sdl_fail(const char *msg) {
  sdl_error(msg);
  exit(EXIT_FAILURE);
}

///
/// Produce messages with values from 0 to count. The last message has tag MSG_TAG_QUIT.
///
/// This function is meant to be run in its own thread by passing it to SDL_CreateThread().
///
/// @param data Pointer to a MessageQueue.
/// @return 0 on success
/// @see consume()
///
static int produce(void *data) {
  extern const int count;

  if (data == NULL) {
    fail("produce failed: data is NULL");
  }

  MessageQueue *queue = (MessageQueue *)data;
  Message msg = {0};
  const char *tagStr = NULL;
  int rc = 0;

  for (intptr_t value = 0; value <= count;) {
    msg.tag = (value < count) ? MSG_TAG_SOME : MSG_TAG_QUIT;
    msg.value = value;
    tagStr = msgq_tagStr(msg.tag);

    rc = msgq_put(queue, &msg);
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
/// Consume messages until a message with tag MSG_TAG_QUIT is received.
///
/// This function is meant to be run on the main thread.
///
/// @param queue Pointer to a MessageQueue.
/// @param out Pointer to a Message.
/// @return 0 when a message with tag MSG_TAG_QUIT is received, 1 otherwise
/// @see produce()
///
static int consume(MessageQueue *queue, Message *out) {
  const int rc = msgq_get(queue, out);
  if (rc < 0) {
    msgq_fail(rc, "msgq_get failed");
  }
  SDL_LogInfo(APP, "Consumed {%s, %" PRIdPTR "}",
              msgq_tagStr(out->tag), out->value);
  return out->tag != MSG_TAG_QUIT;
}

///
/// Initialize SDL and a MessageQueue, run the producer thread, Consume, and clean up.
///
int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
  extern const uint32_t queueCap;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  int rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc < 0) {
    sdl_fail("SDL_Init failed");
  }

  AT_EXIT(SDL_Quit);

  SCOPED_PTR_MessageQueue queue = msgq_create(queueCap);
  if (queue == NULL) {
    fail("msgq_create failed");
  }

  SDL_Thread *producer = SDL_CreateThread(produce, "producer", queue);
  if (producer == NULL) {
    sdl_fail("SDL_CreateThread failed");
  }

  Message msg;
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
