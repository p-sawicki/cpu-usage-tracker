#ifndef CPU_USAGE_TRACKER_QUEUE_H
#define CPU_USAGE_TRACKER_QUEUE_H

#include <errno.h>
#include <pthread.h>

/**
 * @brief Queue node containing generic data.
 * Not to be used directly - see queue_push, queue_pop
 *
 */
typedef struct queue_node_t {
  void *data;
  struct queue_node_t *next;
} queue_node_t;

/**
 * @brief Simple queue for exchanging generic data between threads.
 * Fields should not be accessed directly - use functions declared below.
 *
 */
typedef struct queue_t {
  queue_node_t *first, *last;

  pthread_mutex_t mutex;
  pthread_cond_t cond;

  int exit_flag; // Used for destroying the queue.
} queue_t;

/**
 * @brief Initialize the queue.
 *
 * @param queue Newly created queue.
 * @return int 0 if queue is successfully initialized and can be used.
 * EINVAL if queue is NULL or
 * error code from pthread_mutex_init or pthread_cond_init.
 * If the return value is not 0, using the queue is undefined.
 */
int queue_init(queue_t *queue);

/**
 * @brief Add data to the tail of the queue.
 *
 * @param queue Successfully initialized queue.
 * @param data Data to be added. Data should either be heap-allocated,
 * or ensured that it will not be present in the queue when
 * queue_destroy() is called.
 * @return int 0 if data was added correctly.
 * EINVAL if queue is NULL,
 * ENOMEM if new node could not be created or
 * error code from pthread_mutex_lock, pthread_mutex_unlock
 * or pthread_cond_signal.
 * If the return value is not 0, further usage
 * of the queue is undefined.
 */
int queue_push(queue_t *queue, void *data);

/**
 * @brief Take data from the head of the queue. If the queue is empty,
 * function will block until data is available.
 *
 * @param queue Successfully initialzed queue.
 * @param data Location of returned data. Cannot be NULL.
 * *data may be NULL after calling this function if queue_destroy()
 * was called before data was available.
 * @return int 0 if data was retrieved correctly.
 * EINVAL if queue or data are NULL or
 * error code from pthread_mutex_lock, pthread_cond_wait or
 * pthread_mutex_unlock.
 * If the return value is not 0, further usage
 * of the queue is undefined.
 */
int queue_pop(queue_t *queue, void **data);

/**
 * @brief Release memory owned by queue and make it unusable.
 * If one thread is waiting on queue_pop(), it will be unblocked.
 * If multiple threads are accessing the queue during this call,
 * the result is undefined.
 *
 * @param queue Successfully initialized queue.
 * @return int 0 if queue was destroyed correctly.
 * EINVAL if queue is NULL or
 * error code from pthread_cond_signal, pthread_mutex_lock,
 * pthread_mutex_unlock, pthread_mutex_destroy or pthread_cond_destroy.
 */
int queue_destroy(queue_t *queue);

#endif // CPU_USAGE_TRACKER_QUEUE_H