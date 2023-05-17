///
/// Test basic message queue functionality.
///
/// The producer thread produces messages with values from 0 to count.
/// The consumer consumes messages on the main thread until it receives a
/// message with tag MSG_TAG_QUIT.
///
/// @see msgq_Create()
/// @see msgq_Put()
/// @see msgq_Get()
/// @see msgq_Destroy()
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
static void Fail(const char* msg) {
  SDL_LogError(ERR, "%s", msg);
  exit(EXIT_FAILURE);
}

/// Log a msgq error message and exit.
static void msgq_Fail(int rc, const char* msg) {
  SDL_LogError(ERR, "%s: %s", msg, msgq_Failure(rc));
  exit(EXIT_FAILURE);
}

/// Log a SDL error message and exit.
static void sdl_Fail(const char* msg) {
  sdl_Error(msg);
  exit(EXIT_FAILURE);
}

///
/// Produce messages with values from 0 to count. The last message has tag MSG_TAG_QUIT.
///
/// This function is meant to be run in its own thread by passing it to SDL_CreateThread().
///
/// @param data Pointer to a MessageQueue.
/// @return 0 on success
/// @see Consume()
///
static int Produce(void* data) {
  extern const int count;

  if (data == NULL)
    Fail("Produce failed: data is NULL");

  MessageQueue* queue = (MessageQueue*)data;

  Message msg = {0};
  MessageTag tag = MSG_TAG_NONE;
  const char* tagStr = NULL;

  for (intptr_t value = 0; value <= count;) {
    tag = (value < count) ? MSG_TAG_SOME : MSG_TAG_QUIT;
    tagStr = msgq_MessageTag(tag);

    msg.tag = tag;
    msg.value = value;

    const int rc = msgq_Put(queue, &msg);
    if (rc < 0) {
      msgq_Fail(rc, "msgq_Put failed");
    } else if (rc == 1) {
      SDL_LogDebug(APP, "Produce {%s, %" PRIdPTR "} blocked: retrying",
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
/// @return 0 when a message with tag MSG_TAG_QUIT is received, 1 otherwise
/// @see Produce()
///
static int Consume(MessageQueue* queue) {
  Message msg;

  const int rc = msgq_Get(queue, &msg);
  if (rc < 0)
    msgq_Fail(rc, "msgq_Get failed");

  SDL_LogInfo(APP, "Consumed {%s, %" PRIdPTR "}",
              msgq_MessageTag(msg.tag), msg.value);

  return msg.tag != MSG_TAG_QUIT;
}

///
/// Initialize SDL and a MessageQueue, run the producer thread, Consume, and clean up.
///
int main(_unused_ int argc, _unused_ char* argv[]) {
  extern const uint32_t queueCap;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  int rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc < 0)
    sdl_Fail("SDL_Init failed");

  AT_EXIT(SDL_Quit);

  _cleanup_msgq_ MessageQueue* queue = msgq_Create(queueCap);
  if (queue == NULL)
    Fail("msgq_Create failed");

  SDL_Thread* producer = SDL_CreateThread(Produce, "producer", queue);
  if (producer == NULL)
    sdl_Fail("SDL_CreateThread failed");

  for (;;) {
    rc = Consume(queue);
    if (rc == 0) {
      break;
    } else if (rc < 0) {
      return EXIT_FAILURE;
    }
  }

  SDL_WaitThread(producer, NULL);
  return EXIT_SUCCESS;
}
