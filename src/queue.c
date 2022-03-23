#include <stdlib.h>

#include "queue.h"

int queue_init(queue_t *queue) {
  if (!queue) {
    return EINVAL;
  }

  queue->first = NULL;
  queue->last = NULL;
  queue->exit_flag = 0;

  int res;
  if ((res = pthread_mutex_init(&queue->mutex, NULL))) {
    return res;
  }

  if ((res = pthread_cond_init(&queue->cond, NULL))) {
    pthread_mutex_destroy(&queue->mutex);
    return res;
  }

  return 0;
}

int queue_push(queue_t *queue, void *data) {
  if (!queue) {
    return EINVAL;
  }

  int res;
  if ((res = pthread_mutex_lock(&queue->mutex))) {
    return res;
  }

  queue_node_t *msg = (queue_node_t *)malloc(sizeof(queue_node_t));
  if (!msg) {
    pthread_mutex_unlock(&queue->mutex);
    return ENOMEM;
  }

  msg->data = data;
  msg->next = NULL;

  int signal = 0;
  if (!queue->first) {
    queue->first = msg;
    signal = 1;
  }

  if (queue->last) {
    queue->last->next = msg;
  }
  queue->last = msg;

  /**
   * Queue was empty before this call so there might be a
   * consumer waiting. Send a signal to unblock it.
   */
  if (signal) {
    if ((res = pthread_cond_signal(&queue->cond))) {
      pthread_mutex_unlock(&queue->mutex);
      return res;
    }
  }

  if ((res = pthread_mutex_unlock(&queue->mutex))) {
    return res;
  }

  return 0;
}

int queue_pop(queue_t *queue, void **data) {
  if (!queue || !data) {
    return EINVAL;
  }

  int res;
  if ((res = pthread_mutex_lock(&queue->mutex))) {
    return res;
  }

  while (!queue->first && !queue->exit_flag) {
    if ((res = pthread_cond_wait(&queue->cond, &queue->mutex))) {
      pthread_mutex_unlock(&queue->mutex);
      return res;
    }
  }

  if (queue->first) {
    *data = queue->first->data;

    queue_node_t *first = queue->first;
    queue->first = queue->first->next;
    free(first);

    if (!queue->first) {
      queue->last = NULL;
    }
  } else { // Possible if thread was unblocked by queue_destroy().
    *data = NULL;
  }

  if ((res = pthread_mutex_unlock(&queue->mutex))) {
    return res;
  }

  return 0;
}

int queue_destroy(queue_t *queue) {
  if (!queue) {
    return EINVAL;
  }

  queue->exit_flag = 1;
  int res;
  // Unblock consumer thread if it currently waiting. If it
  // is unblocked, the consumer thread takes the mutex.
  if ((res = pthread_cond_signal(&queue->cond))) {
    return res;
  }

  if ((res = pthread_mutex_lock(&queue->mutex))) {
    return res;
  }

  while (queue->first) {
    queue_node_t *first = queue->first;
    queue->first = first->next;
    free(first->data);
    free(first);
  }

  if ((res = pthread_mutex_unlock(&queue->mutex))) {
    return res;
  }

  if ((res = pthread_mutex_destroy(&queue->mutex))) {
    return res;
  }

  if ((res = pthread_cond_destroy(&queue->cond))) {
    return res;
  }

  return 0;
}