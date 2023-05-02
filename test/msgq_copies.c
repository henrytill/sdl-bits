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
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL.h>

#include "macro.h"
#include "msgq.h"

#define LOGMSG(msg)                               \
  do {                                            \
    SDL_LogInfo(APP, "%s: %s{%s, %" PRIdPTR "}",  \
                __func__, #msg,                   \
                msgq_tagstr(msg.tag), msg.value); \
  } while (0)

#define CHECKMSG(msg, extag, exvalue)                            \
  do {                                                           \
    if (msg.tag != extag || msg.value != exvalue) {              \
      SDL_LogError(ERR, "%s: %s{%s, %" PRIdPTR "} != {%s, %ld}", \
                   __func__, #msg,                               \
                   msgq_tagstr(msg.tag), msg.value,              \
                   msgq_tagstr(extag), exvalue);                 \
      exit(EXIT_FAILURE);                                        \
    }                                                            \
  } while (0);

/** Log categories to use with SDL logging functions. */
enum LogCategory {
  APP = SDL_LOG_CATEGORY_CUSTOM,
  ERR,
};

/** Delay before consuming messages. */
static const uint32_t delay = 2000U;

/** Capacity of the MessageQueue. */
static const uint32_t qcap = 1U;

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
    if (rc < 0)
      qfail(rc, "msgq_put failed");
  }
  LOGMSG(m);

  m.tag = SOME;
  m.value = 0;
  for (int rc = 1; rc == 1;) {
    rc = msgq_put(queue, &m);
    if (rc < 0)
      qfail(rc, "msgq_put failed");
  }
  LOGMSG(m);

  m.tag = SOME;
  m.value = 1;
  for (int rc = 1; rc == 1;) {
    rc = msgq_put(queue, &m);
    if (rc < 0)
      qfail(rc, "msgq_put failed");
  }
  LOGMSG(m);

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
  extern const uint32_t delay;
  struct Message a;
  struct Message b;
  struct Message c;

  SDL_LogInfo(APP, "%s: pausing for %d...", __func__, delay);
  SDL_Delay(delay);

  msgq_get(queue, (void *)&a);
  LOGMSG(a);
  CHECKMSG(a, SOME, 42l);

  msgq_get(queue, (void *)&b);
  LOGMSG(b);
  CHECKMSG(a, SOME, 42l);
  CHECKMSG(b, SOME, 0l);

  msgq_get(queue, (void *)&c);
  LOGMSG(c);
  CHECKMSG(a, SOME, 42l);
  CHECKMSG(b, SOME, 0l);
  CHECKMSG(c, SOME, 1l);

  return 0;
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
  if (rc != 0)
    sdlfail("SDL_Init failed");

  exitwith(SDL_Quit);

  rc = msgq_init(&q, qcap);
  if (rc != 0)
    qfail(rc, "msgq_init failed");

  exitwith(qfinish);

  producer = SDL_CreateThread(produce, "producer", (void *)&q);
  if (producer == NULL)
    sdlfail("SDL_CreateThread failed");

  if (consume(&q) != 0)
    return EXIT_FAILURE;

  SDL_WaitThread(producer, NULL);
  return EXIT_SUCCESS;
}
