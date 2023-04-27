#include <stdio.h>

#include <SDL.h>

#include <queue.h>

#define forever for (;;)

enum {
  APP = SDL_LOG_CATEGORY_CUSTOM,
  ERR,
};

struct Message {
  enum Tag {
    NONE = 0,
    SOME = 1,
  } tag;
  intptr_t value;
};

static const char *const tagdesc[] = {
  [NONE] = "NONE",
  [SOME] = "SOME",
};

static const int MAX = 200;

static struct Queue q;

static int produce(void *data) {
  int rc;

  if (data == NULL) {
    SDL_LogError(ERR, "produce failed: data is NULL\n");
    return 1;
  }
  struct Queue *queue = (struct Queue *)data;

  for (intptr_t value = 0; value <= MAX;) {
    const enum Tag tag = (value < MAX) ? SOME : NONE;

    struct Message *msg = malloc(sizeof(*msg));
    if (msg == NULL) {
      SDL_LogError(ERR, "produce {%s, %ld} failed: malloc failed\n", tagdesc[tag], value);
      return 1;
    }
    msg->tag = tag;
    msg->value = value;

    rc = queue_put(queue, (void *)msg);
    if (rc == 1) {
      SDL_LogDebug(APP, "produce {%s, %ld} blocked: retrying\n", tagdesc[tag], value);
      continue;
    } else if (rc < 0) {
      SDL_LogError(ERR, "produce {%s, %ld} failed: %s\n", tagdesc[tag], value, queue_error(rc));
      return 1;
    } else {
      SDL_LogInfo(APP, "produced {%s, %ld}\n", tagdesc[tag], value);
      value += 1;
    }
  }

  return 0;
}

static int consume(struct Queue *q) {
  int ret = 1;
  int rc;
  struct Message *msg = NULL;

  rc = queue_get(q, (void *)&msg);
  if (rc < 0) {
    SDL_LogError(ERR, "consume failed: %s\n", queue_error(rc));
    return -1;
  }
  if (msg == NULL) {
    SDL_LogError(ERR, "consume failed: msg is NULL\n");
    return -1;
  }
  if (msg->tag == NONE) {
    ret = 0;
  }

  SDL_LogInfo(APP, "consumed {%s, %ld}\n", tagdesc[msg->tag], msg->value);
  free(msg);
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

  rc = queue_init(&q, 4);
  if (rc != 0) {
    SDL_LogError(ERR, "queue_init failed: %s\n", queue_error(rc));
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
  queue_finish(&q);
out0:
  SDL_Quit();
  return ret;
}
