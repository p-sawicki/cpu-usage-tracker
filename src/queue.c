#include <stdlib.h>

#include "queue.h"

int queue_init(queue_t *queue) {
  if (NULL == queue) {
    return EINVAL;
  }

  queue->first = NULL;
  queue->last = NULL;
  queue->exit_flag = 0;

  int res;
  if (0 != (res = pthread_mutex_init(&queue->mutex, NULL))) {
    return res;
  }

  if (0 != (res = pthread_cond_init(&queue->cond, NULL))) {
    pthread_mutex_destroy(&queue->mutex);
    return res;
  }

  return 0;
}

int queue_push(queue_t *queue, void *data) {
  if (NULL == queue || 0 != queue->exit_flag) {
    return EINVAL;
  }

  int res;
  if (0 != (res = pthread_mutex_lock(&queue->mutex))) {
    return res;
  }

  struct queue_node_t *msg =
      (struct queue_node_t *)malloc(sizeof(struct queue_node_t));
  if (NULL == msg) {
    pthread_mutex_unlock(&queue->mutex);
    return ENOMEM;
  }

  msg->data = data;
  msg->next = NULL;

  int signal = 0;
  if (NULL == queue->first) {
    queue->first = msg;
    signal = 1;
  }

  if (NULL != queue->last) {
    queue->last->next = msg;
  }
  queue->last = msg;

  /**
   * Queue was empty before this call so there might be a
   * consumer waiting. Send a signal to unblock it.
   */
  if (0 != signal) {
    if ((res = pthread_cond_signal(&queue->cond))) {
      pthread_mutex_unlock(&queue->mutex);
      return res;
    }
  }

  if (0 != (res = pthread_mutex_unlock(&queue->mutex))) {
    return res;
  }

  return 0;
}

int queue_pop(queue_t *queue, void **data, time_t timeout) {
  if (NULL == queue || NULL == data || 0 != queue->exit_flag) {
    return EINVAL;
  }

  int res;
  if (0 != (res = pthread_mutex_lock(&queue->mutex))) {
    return res;
  }

  struct timespec ts;
  if (0 != (res = clock_gettime(CLOCK_REALTIME, &ts))) {
    return res;
  }
  ts.tv_sec += timeout;

  while (NULL == queue->first && 0 == queue->exit_flag) {
    if (0 == timeout) {
      if (0 != (res = pthread_cond_wait(&queue->cond, &queue->mutex))) {
        pthread_mutex_unlock(&queue->mutex);
        return res;
      }
      break;
    }

    res = pthread_cond_timedwait(&queue->cond, &queue->mutex, &ts);
    if (ETIMEDOUT == res) {
      break;
    }
    if (0 != res) {
      pthread_mutex_unlock(&queue->mutex);
      return res;
    }
  }

  if (NULL != queue->first) {
    *data = queue->first->data;

    struct queue_node_t *first = queue->first;
    queue->first = queue->first->next;
    free(first);

    if (NULL == queue->first) {
      queue->last = NULL;
    }
  } else { // Possible if thread was unblocked by queue_destroy().
    *data = NULL;
  }

  if (0 != (res = pthread_mutex_unlock(&queue->mutex))) {
    return res;
  }

  return 0;
}

int queue_destroy(queue_t *queue) {
  if (NULL == queue || 0 != queue->exit_flag) {
    return EINVAL;
  }

  queue->exit_flag = 1;
  int res;

  // Unblock consumer threads if they are currently waiting. If any
  // is unblocked, the consumer thread takes the mutex and this blocks on it.
  if (0 != (res = pthread_cond_broadcast(&queue->cond))) {
    return res;
  }

  if (0 != (res = pthread_mutex_lock(&queue->mutex))) {
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
    return res;
  }

  // Wait until all consumer threads are finished in case this thread won the
  // mutex before any of them. Should not take long so we use a spinlock.
  while (EBUSY == (res = pthread_mutex_destroy(&queue->mutex)))
    ;
  if (0 != res) {
    return res;
  }

  if (0 != (res = pthread_cond_destroy(&queue->cond))) {
    return res;
  }

  return 0;
}