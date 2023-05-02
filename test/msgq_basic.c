/**
 * Test basic message queue functionality.
 *
 * The producer thread produces messages with values from 0 to count.
 * The consumer consumes messages on the main thread until it receives a
 * message with tag NONE.
 *
 * @see msgq_init()
 * @see msgq_put()
 * @see msgq_get()
 * @see msgq_destroy()
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL.h>

#include "macro.h"
#include "msgq.h"

/** Log categories to use with SDL logging functions. */
enum LogCategory {
  APP = SDL_LOG_CATEGORY_CUSTOM,
  ERR,
};

/** Maximum value to produce. */
static const int count = 100;

/** Capacity of the MessageQueue. */
static const uint32_t qcap = 4;

/** MessageQueue for testing. */
static struct MessageQueue q;

/**
 * Call msgq_finish() on q.
 *
 * @see msgq_finish()
 */
static void qfinish(void) {
  extern struct MessageQueue q;
  msgq_finish(&q);
}

/** Log an error message and exits. */
static void fail(const char *msg) {
  SDL_LogError(ERR, "%s", msg);
  exit(EXIT_FAILURE);
}

/** Log a msgq error message and exits. */
static void qfail(int err, const char *msg) {
  SDL_LogError(ERR, "%s: %s", msg, msgq_errorstr(err));
  exit(EXIT_FAILURE);
}

/** Log a SDL error message and exits. */
static void sdlfail(const char *msg) {
  const char *err = SDL_GetError();
  if (strlen(err) != 0)
    SDL_LogError(ERR, "%s (%s)", msg, err);
  else
    SDL_LogError(ERR, "%s", msg);
  exit(EXIT_FAILURE);
}

/**
 * Produce messages with values from 0 to count. The last message has
 * tag NONE.
 *
 * This function is meant to be run in its own thread by passing it to SDL_CreateThread().
 *
 * @param data Pointer to a MessageQueue.
 * @return 0 on success
 * @see consume()
 */
static int produce(void *data) {
  extern const int count;
  int rc;
  struct Message msg;
  enum MessageTag tag;
  const char *tagstr = NULL;

  if (data == NULL)
    fail("produce failed: data is NULL");

  struct MessageQueue *queue = (struct MessageQueue *)data;

  for (intptr_t value = 0; value <= count;) {
    tag = (value < count) ? SOME : NONE;
    tagstr = msgq_tagstr(tag);

    msg.tag = tag;
    msg.value = value;

    rc = msgq_put(queue, (void *)&msg);
    if (rc < 0) {
      qfail(rc, "msgq_put failed");
    } else if (rc == 1) {
      SDL_LogDebug(APP, "produce {%s, %" PRIdPTR "} blocked: retrying",
                   tagstr, value);
      continue;
    } else {
      SDL_LogInfo(APP, "Produced {%s, %" PRIdPTR "}",
                  tagstr, value);
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
 * @return 0 when a message with tag NONE is received, 1 when a message with tag SOME is received,
 * @see produce()
 */
static int consume(struct MessageQueue *queue) {
  struct Message msg;

  const int rc = msgq_get(queue, (void *)&msg);
  if (rc < 0)
    qfail(rc, "msgq_get failed");

  SDL_LogInfo(APP, "Consumed {%s, %" PRIdPTR "}",
              msgq_tagstr(msg.tag), msg.value);

  return msg.tag != NONE;
}

/**
 * Initialize SDL and a MessageQueue, run the producer thread, consume, and clean up.
 */
int main(_unused_ int argc, _unused_ char *argv[]) {
  extern struct MessageQueue q;
  extern const uint32_t qcap;

  int rc;
  SDL_Thread *producer;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc < 0)
    sdlfail("SDL_Init failed");

  exitwith(SDL_Quit);

  rc = msgq_init(&q, qcap);
  if (rc < 0)
    qfail(rc, "msgq_init failed");

  exitwith(qfinish);

  producer = SDL_CreateThread(produce, "producer", &q);
  if (producer == NULL)
    sdlfail("SDL_CreateThread failed");

  for (;;) {
    rc = consume(&q);
    if (rc == 0) {
      break;
    } else if (rc < 0) {
      return EXIT_FAILURE;
    }
  }

  SDL_WaitThread(producer, NULL);
  return EXIT_SUCCESS;
}
