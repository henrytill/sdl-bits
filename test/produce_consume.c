#include <stdio.h>

#include <SDL.h>

#include "msgq.h"

#define forever for (;;)

enum {
  APP = SDL_LOG_CATEGORY_CUSTOM,
  ERR,
};

static const int COUNT_MAX = 1000;
static const uint32_t Q_CAPACITY = 10;

static struct MessageQueue q;

static int produce(void *data) {
  int rc;
  struct Message msg;
  enum Tag tag;
  const char *tagstr = NULL;

  if (data == NULL) {
    SDL_LogError(ERR, "produce failed: data is NULL\n");
    return 1;
  }
  struct MessageQueue *queue = (struct MessageQueue *)data;

  for (intptr_t value = 0; value <= COUNT_MAX;) {
    tag = (value < COUNT_MAX) ? SOME : NONE;

    msg.tag = tag;
    msg.value = value;
    tagstr = msgq_tagstr(&msg);

    rc = msgq_put(queue, (void *)&msg);
    if (rc == 1) {
      SDL_LogDebug(APP, "produce {%s, %ld} blocked: retrying\n", tagstr, value);
      continue;
    } else if (rc < 0) {
      SDL_LogError(ERR, "produce {%s, %ld} failed: %s\n", tagstr, value, msgq_errorstr(rc));
      return 1;
    } else {
      SDL_LogInfo(APP, "produced {%s, %ld}\n", tagstr, value);
      value += 1;
    }
  }

  return 0;
}

static int consume(struct MessageQueue *queue) {
  int ret = 1;
  int rc;
  struct Message msg;

  rc = msgq_get(queue, (void *)&msg);
  if (rc < 0) {
    SDL_LogError(ERR, "consume failed: %s\n", msgq_errorstr(rc));
    return -1;
  }
  if (msg.tag == NONE) {
    ret = 0;
  }
  SDL_LogInfo(APP, "consumed {%s, %ld}\n", msgq_tagstr(&msg), msg.value);
  return ret;
}

int main(int argc, char *argv[]) {
  int ret = EXIT_FAILURE;
  int rc;
  SDL_Thread *producer;

  (void)argc;
  (void)argv;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc != 0) {
    SDL_LogError(ERR, "SDL_Init failed\n");
    return EXIT_FAILURE;
  }

  rc = msgq_init(&q, Q_CAPACITY);
  if (rc != 0) {
    SDL_LogError(ERR, "msgq_init failed: %s\n", msgq_errorstr(rc));
    ret = EXIT_FAILURE;
    goto out0;
  }

  producer = SDL_CreateThread(produce, "producer", &q);
  if (producer == NULL) {
    SDL_LogError(ERR, "SDL_CreateThread failed: %s\n", SDL_GetError());
    ret = EXIT_FAILURE;
    goto out1;
  }

  forever {
    rc = consume(&q);
    if (rc == 0) {
      break;
    } else if (rc < 0) {
      ret = EXIT_FAILURE;
      goto out1;
    }
  }

  ret = EXIT_SUCCESS;
  SDL_WaitThread(producer, NULL);
  SDL_LogInfo(APP, "goodbye...\n");
out1:
  msgq_finish(&q);
out0:
  SDL_Quit();
  return ret;
}
