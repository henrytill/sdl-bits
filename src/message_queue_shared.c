#include "message_queue.h"

#include <stddef.h>

const char *message_queue_failure_str(enum message_queue_failure failure)
{
    switch (failure) {
    case MSGQ_FAILURE_NULL_POINTER:
        return "NULL pointer";
    case MSGQ_FAILURE_MALLOC:
        return "malloc failed";
    case MSGQ_FAILURE_SEM_CREATE:
        return "create semaphore failed";
    case MSGQ_FAILURE_SEM_POST:
        return "post sempahore failed";
    case MSGQ_FAILURE_SEM_TRY_WAIT:
        return "try-wait sempahore failed";
    case MSGQ_FAILURE_SEM_WAIT:
        return "wait semaphore failed";
    case MSGQ_FAILURE_MUTEX_CREATE:
        return "create mutex failed";
    case MSGQ_FAILURE_MUTEX_LOCK:
        return "lock mutex failed";
    case MSGQ_FAILURE_MUTEX_UNLOCK:
        return "unlock mutex failed";
    case MSGQ_FAILURE_MIN:
    default:
        return NULL;
    }
}

const char *message_tag_str(enum message_tag tag)
{
    switch (tag) {
    case MSG_TAG_NONE:
        return "NONE";
    case MSG_TAG_SOME:
        return "SOME";
    case MSG_TAG_QUIT:
        return "QUIT";
    default:
        return NULL;
    }
}
