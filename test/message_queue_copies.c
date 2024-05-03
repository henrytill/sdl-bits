/// Test that values are copied into and out of the message queue.
///
/// The producer thread produces messages with values 42, 0, and 1.
/// The consumer consumes messages on the main thread after a delay and checks
/// their values.
///
/// @see message_queue_create()
/// @see message_queue_put()
/// @see message_queue_get()
/// @see message_queue_destroy()
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "macro.h"
#include "message_queue.h"
#include "prelude_sdl.h"

#define LOG(_msg) ({                                        \
    struct message __msg = (_msg);                          \
    SDL_LogInfo(APP, "%s: %s{%s, %" PRIdPTR "}",            \
                __func__, #_msg,                            \
                message_queue_tag(__msg.tag), __msg.value); \
})

#define CHECK(_msg, _tag, _value) ({                               \
    struct message __msg = (_msg);                                 \
    int __tag = (_tag);                                            \
    typeof(_value) __value = (_value);                             \
    if (__msg.tag != __tag || __msg.value != __value) {            \
        SDL_LogError(ERR, "%s: %s{%s, %" PRIdPTR "} != {%s, %ld}", \
                     __func__, #_msg,                              \
                     message_queue_tag(__msg.tag), __msg.value,    \
                     message_queue_tag(__tag), __value);           \
        exit(EXIT_FAILURE);                                        \
    }                                                              \
});

/// Delay before consuming messages.
static const uint32_t DELAY = 2000U;

/// Capacity of the message_queue.
static const uint32_t QUEUE_CAP = 1U;

/// Log a message_queue error message and exit.
static void message_queue_fail(int rc, const char *msg)
{
    SDL_LogError(ERR, "%s: %s", msg, message_queue_failure(rc));
    exit(EXIT_FAILURE);
}

/// Log a SDL error message and exit.
static void sdl_fail(const char *msg)
{
    log_sdl_error(msg);
    exit(EXIT_FAILURE);
}

/// Produce messages.
///
/// This function is meant to be run in its own thread by passing it to SDL_CreateThread().
///
/// @param data Pointer to a message_queue.
/// @return 0 on success, 1 on failure.
/// @see consume()
static int produce(void *data)
{
    struct message_queue *queue = data;
    struct message msg = {.tag = MSG_TAG_SOME, .value = 42};

    for (int rc = 1; rc == 1;) {
        rc = message_queue_put(queue, &msg);
        if (rc < 0) {
            message_queue_fail(rc, "message_queue_put failed");
        }
    }
    LOG(msg);

    msg.tag = MSG_TAG_SOME;
    msg.value = 0;
    for (int rc = 1; rc == 1;) {
        rc = message_queue_put(queue, &msg);
        if (rc < 0) {
            message_queue_fail(rc, "message_queue_put failed");
        }
    }
    LOG(msg);

    msg.tag = MSG_TAG_SOME;
    msg.value = 1;
    for (int rc = 1; rc == 1;) {
        rc = message_queue_put(queue, &msg);
        if (rc < 0) {
            message_queue_fail(rc, "message_queue_put failed");
        }
    }
    LOG(msg);

    return 0;
}

/// Consume messages produced by produce() after a delay and check their values.
///
/// This function is meant to be run in the main thread.
///
/// @param queue Pointer to a message_queue.
/// @return 0 on success, 1 on failure.
/// @see produce()
static int consume(struct message_queue *queue)
{
    extern const uint32_t DELAY;

    struct message a = {0};
    struct message b = {0};
    struct message c = {0};

    SDL_LogInfo(APP, "%s: pausing for %d...", __func__, DELAY);
    SDL_Delay(DELAY);

    message_queue_get(queue, &a);
    LOG(a);
    CHECK(a, MSG_TAG_SOME, 42L);

    message_queue_get(queue, &b);
    LOG(b);
    CHECK(a, MSG_TAG_SOME, 42L);
    CHECK(b, MSG_TAG_SOME, 0L);

    message_queue_get(queue, &c);
    LOG(c);
    CHECK(a, MSG_TAG_SOME, 42L);
    CHECK(b, MSG_TAG_SOME, 0L);
    CHECK(c, MSG_TAG_SOME, 1L);

    return 0;
}

/// Initialize SDL and a message_queue, run the producer thread, consume,
/// and clean up.
int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
    extern const uint32_t QUEUE_CAP;

    int ret = EXIT_FAILURE;

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

    int rc = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);
    if (rc != 0) {
        sdl_fail("SDL_Init failed");
    }

    AT_EXIT(SDL_Quit);

    struct message_queue *queue = message_queue_create(QUEUE_CAP);
    if (queue == NULL) {
        log_sdl_error("message_queue_create failed");
    }

    SDL_Thread *producer = SDL_CreateThread(produce, "producer", queue);
    if (producer == NULL) {
        log_sdl_error("SDL_CreateThread failed");
        goto out_destroy_queue;
    }

    if (consume(queue) != 0) {
        (void)fprintf(stderr, "consume failed");
        goto out_destroy_queue;
    }

    SDL_WaitThread(producer, NULL);

    ret = EXIT_SUCCESS;
out_destroy_queue:
    message_queue_destroy(queue);
    return ret;
}
