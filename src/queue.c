#include <stdlib.h>

#include "queue.h"

int queue_init(queue_t *queue) {
  int res;
  if (NULL == queue) {
    return EINVAL;
  }

  queue->first = NULL;
  queue->last = NULL;
  queue->state = UNINITIALIZED;

  if (0 != (res = pthread_mutex_init(&queue->mutex, NULL))) {
    return res;
  }

  if (0 != (res = pthread_cond_init(&queue->cond, NULL))) {
    pthread_mutex_destroy(&queue->mutex);
    return res;
  }

  queue->state = OK;
  return 0;
}

int queue_push(queue_t *queue, void *data) {
  int res, signal = 0;
  struct queue_node_t *node;
  if (NULL == queue || OK != queue->state) {
    return EINVAL;
  }

  if (0 != (res = pthread_mutex_lock(&queue->mutex))) {
    queue->state = INVALID;
    return res;
  }

  node = (struct queue_node_t *)malloc(sizeof(struct queue_node_t));
  if (NULL == node) {
    pthread_mutex_unlock(&queue->mutex);
    queue->state = INVALID;
    return ENOMEM;
  }

  node->data = data;
  node->next = NULL;

  if (NULL == queue->first) {
    queue->first = node;
    signal = 1;
  }

  if (NULL != queue->last) {
    queue->last->next = node;
  }
  queue->last = node;

  /**
   * Queue was empty before this call so there might be a
   * consumer waiting. Send a signal to unblock it.
   */
  if (0 != signal) {
    if ((res = pthread_cond_signal(&queue->cond))) {
      pthread_mutex_unlock(&queue->mutex);
      queue->state = INVALID;
      return res;
    }
  }

  if (0 != (res = pthread_mutex_unlock(&queue->mutex))) {
    queue->state = INVALID;
    return res;
  }

  return 0;
}

int queue_pop(queue_t *queue, void **data, time_t timeout) {
  int res;
  struct queue_node_t *first;
  struct timespec ts;
  if (NULL == queue || NULL == data || OK != queue->state) {
    return EINVAL;
  }

  if (0 != (res = pthread_mutex_lock(&queue->mutex))) {
    queue->state = INVALID;
    return res;
  }

  if (0 != (res = clock_gettime(CLOCK_REALTIME, &ts))) {
    queue->state = INVALID;
    return res;
  }
  ts.tv_sec += timeout;

  while (NULL == queue->first && OK == queue->state) {
    if (0 == timeout) {
      if (0 != (res = pthread_cond_wait(&queue->cond, &queue->mutex))) {
        pthread_mutex_unlock(&queue->mutex);
        queue->state = INVALID;
        return res;
      }
      continue; // Check conditions again in case of spurious wake-ups.
    }

    res = pthread_cond_timedwait(&queue->cond, &queue->mutex, &ts);
    if (ETIMEDOUT == res) {
      break;
    }
    if (0 != res) {
      pthread_mutex_unlock(&queue->mutex);
      queue->state = INVALID;
      return res;
    }
  }

  if (NULL != queue->first) {
    *data = queue->first->data;

    first = queue->first;
    queue->first = queue->first->next;
    free(first);

    if (NULL == queue->first) {
      queue->last = NULL;
    }
  } else { // Possible if thread was unblocked by queue_destroy() or timed out.
    *data = NULL;
  }

  if (0 != (res = pthread_mutex_unlock(&queue->mutex))) {
    queue->state = INVALID;
    return res;
  }

  return 0;
}

int queue_destroy(queue_t *queue) {
  int res;
  if (NULL == queue || !(OK == queue->state || INVALID == queue->state)) {
    return EINVAL;
  }

  queue->state = DESTROYED;

  // Unblock consumer threads if they are currently waiting. If any
  // is unblocked, the consumer thread takes the mutex and this blocks on it.
  if (0 != (res = pthread_cond_broadcast(&queue->cond))) {
    queue->state = INVALID;
    return res;
  }

  if (0 != (res = pthread_mutex_lock(&queue->mutex))) {
    queue->state = INVALID;
    return res;
  }

  // Now it is guaranteed that no other thread can access these values
  // so they are safe to free().
  while (NULL != queue->first) {
    struct queue_node_t *first = queue->first;
    queue->first = first->next;
    free(first->data);
    free(first);
  }

  if (0 != (res = pthread_mutex_unlock(&queue->mutex))) {
    queue->state = INVALID;
    return res;
  }

  // Wait until all consumer threads are finished in case this thread won the
  // mutex before any of them. Should not take long so we use a spinlock.
  while (EBUSY == (res = pthread_mutex_destroy(&queue->mutex)))
    ;
  if (0 != res) {
    queue->state = INVALID;
    return res;
  }

  if (0 != (res = pthread_cond_destroy(&queue->cond))) {
    queue->state = INVALID;
    return res;
  }

  queue->state = DESTROYED;
  return 0;
}
