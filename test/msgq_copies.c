/**
 * Test that values are copied into and out of the message queue.
 *
 * The producer thread produces messages with values 42, 0, and 1.
 * The consumer consumes messages on the main thread after a delay and checks
 * their values.
 *
 * @see msgq_put()
 * @see msgq_get()
 */
#include <stdio.h>

#include <SDL.h>

#include "msgq.h"

#define logmsg(msg)                                     \
  do {                                                  \
    SDL_LogInfo(APP, "%s: %s{%s, %ld}", __func__, #msg, \
                msgq_tagstr(msg.tag), msg.value);       \
  } while (0)

#define checkmsg(msg, extag, exvalue)                                   \
  do {                                                                  \
    if (msg.tag != extag || msg.value != exvalue) {                     \
      SDL_LogError(ERR, "%s: %s{%s, %ld} != {%s, %ld}", __func__, #msg, \
                   msgq_tagstr(msg.tag), msg.value,                     \
                   msgq_tagstr(extag), exvalue);                        \
      return 1;                                                         \
    }                                                                   \
  } while (0);

/** Log categories to use with SDL logging functions. */
enum LogCategory {
  APP = SDL_LOG_CATEGORY_CUSTOM,
  ERR,
};

/** Delay before consuming messages. */
static const uint32_t DELAY = 2000U;

/** Capacity of the MessageQueue. */
static const uint32_t Q_CAPACITY = 1U;

/** MessageQueue for testing. */
static struct MessageQueue q;

/**
 * Produce messages.
 *
 * This function is meant to be run in its own thread by passing it to SDL_CreateThread().
 *
 * @param data Pointer to a MessageQueue.
 * @return 0 on success, 1 on failure.
 * @see consume()
 */
static int produce(void *data) {
  struct MessageQueue *queue = (struct MessageQueue *)data;
  struct Message m;

  m.tag = SOME;
  m.value = 42;
  for (int rc = 1; rc == 1;) {
    rc = msgq_put(queue, &m);
  }
  logmsg(m);

  m.tag = SOME;
  m.value = 0;
  for (int rc = 1; rc == 1;) {
    rc = msgq_put(queue, &m);
  }
  logmsg(m);

  m.tag = SOME;
  m.value = 1;
  for (int rc = 1; rc == 1;) {
    rc = msgq_put(queue, &m);
  }
  logmsg(m);

  return 0;
}

/**
 * Consume messages produced by produce() after a delay and check their values.
 *
 * This function is meant to be run in the main thread.
 *
 * @param queue Pointer to a MessageQueue.
 * @return 0 on success, 1 on failure.
 * @see produce()
 */
static int consume(struct MessageQueue *queue) {
  struct Message a;
  struct Message b;
  struct Message c;

  SDL_LogInfo(APP, "%s: pausing for %d...", __func__, DELAY);
  SDL_Delay(DELAY);

  msgq_get(queue, (void *)&a);
  logmsg(a);
  checkmsg(a, SOME, 42l);

  msgq_get(queue, (void *)&b);
  logmsg(b);
  checkmsg(a, SOME, 42l);
  checkmsg(b, SOME, 0l);

  msgq_get(queue, (void *)&c);
  logmsg(c);
  checkmsg(a, SOME, 42l);
  checkmsg(b, SOME, 0l);
  checkmsg(c, SOME, 1l);

  return 0;
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

  producer = SDL_CreateThread(produce, "producer", (void *)&q);
  if (producer == NULL) {
    SDL_LogError(ERR, "SDL_CreateThread failed");
    goto out1;
  }

  if (consume(&q) != 0) {
    goto out1;
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
