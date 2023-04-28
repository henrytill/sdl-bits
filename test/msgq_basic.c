/**
 * Test basic message queue functionality.
 *
 * The producer thread produces messages with values from 0 to COUNT_MAX.
 * The consumer consumes messages on the main thread until it receives a
 * message with tag NONE.
 *
 * @see msgq_init()
 * @see msgq_put()
 * @see msgq_get()
 * @see msgq_destroy()
 */
#include <stdio.h>

#include <SDL.h>

#include "msgq.h"

#define forever for (;;)

/** Log categories to use with SDL logging functions. */
enum LogCategory {
  APP = SDL_LOG_CATEGORY_CUSTOM,
  ERR,
};

/** Maximum value to produce. */
static const int COUNT_MAX = 100;

/** Capacity of the MessageQueue. */
static const uint32_t Q_CAPACITY = 4;

/** MessageQueue for testing. */
static struct MessageQueue q;

/**
 * Produce messages with values from 0 to COUNT_MAX. The last message has
 * tag NONE.
 *
 * This function is meant to be run in its own thread by passing it to SDL_CreateThread().
 *
 * @param data Pointer to a MessageQueue.
 * @return 0 on success, 1 on failure.
 * @see consume()
 */
static int produce(void *data) {
  int rc;
  struct Message msg;
  enum MessageTag tag;
  const char *tagstr = NULL;

  if (data == NULL) {
    SDL_LogError(ERR, "produce failed: data is NULL");
    return 1;
  }
  struct MessageQueue *queue = (struct MessageQueue *)data;

  for (intptr_t value = 0; value <= COUNT_MAX;) {
    tag = (value < COUNT_MAX) ? SOME : NONE;
    tagstr = msgq_tagstr(tag);

    msg.tag = tag;
    msg.value = value;

    rc = msgq_put(queue, (void *)&msg);
    if (rc == 1) {
      SDL_LogDebug(APP, "produce {%s, %ld} blocked: retrying", tagstr, value);
      continue;
    } else if (rc < 0) {
      SDL_LogError(ERR, "produce {%s, %ld} failed: %s", tagstr, value, msgq_errorstr(rc));
      return 1;
    } else {
      SDL_LogInfo(APP, "produced {%s, %ld}", tagstr, value);
      value += 1;
    }
  }

  return 0;
}

/**
 * Consume messages until a message with tag NONE is received.
 *
 * This function is meant to be run on the main thread.
 *
 * @param queue Pointer to a MessageQueue.
 * @return 0 on success, 1 on failure.
 * @see produce()
 */
static int consume(struct MessageQueue *queue) {
  int ret = 1;
  int rc;
  struct Message msg;

  rc = msgq_get(queue, (void *)&msg);
  if (rc < 0) {
    SDL_LogError(ERR, "consume failed: %s", msgq_errorstr(rc));
    return -1;
  }
  if (msg.tag == NONE) {
    ret = 0;
  }
  SDL_LogInfo(APP, "consumed {%s, %ld}", msgq_tagstr(msg.tag), msg.value);
  return ret;
}

/**
 * Initialize SDL and a MessageQueue, run the producer thread, consume, and clean up.
 */
int main(int argc, char *argv[]) {
  int ret = EXIT_FAILURE;
  int rc;
  SDL_Thread *producer;

  (void)argc;
  (void)argv;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc != 0) {
    SDL_LogError(ERR, "SDL_Init failed");
    return EXIT_FAILURE;
  }

  rc = msgq_init(&q, Q_CAPACITY);
  if (rc != 0) {
    SDL_LogError(ERR, "msgq_init failed: %s", msgq_errorstr(rc));
    goto out0;
  }

  producer = SDL_CreateThread(produce, "producer", &q);
  if (producer == NULL) {
    SDL_LogError(ERR, "SDL_CreateThread failed: %s", SDL_GetError());
    goto out1;
  }

  forever {
    rc = consume(&q);
    if (rc == 0) {
      break;
    } else if (rc < 0) {
      goto out1;
    }
  }

  ret = EXIT_SUCCESS;
  SDL_WaitThread(producer, NULL);
  SDL_LogInfo(APP, "SDL_WaitThread");
out1:
  msgq_finish(&q);
  SDL_LogInfo(APP, "msgq_finish");
out0:
  SDL_Quit();
  printf("SDL_Quit\n");
  return ret;
}
