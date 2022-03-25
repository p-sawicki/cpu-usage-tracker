#ifndef CPU_USAGE_TRACKER_QUEUE_H
#define CPU_USAGE_TRACKER_QUEUE_H

#include <errno.h>
#include <pthread.h>

/**
 * @brief Queue node containing generic data.
 * Not to be used directly - see queue_push, queue_pop
 *
 */
struct queue_node_t {
  void *data;
  struct queue_node_t *next;
};

/**
 * @brief State of the queue used to prevent invalid access.
 *
 */
enum queue_state { OK = 0, UNINITIALIZED, INVALID, DESTROYED };

/**
 * @brief Simple queue for exchanging generic data between threads.
 * Fields should not be accessed directly - use functions declared below.
 *
 */
typedef struct queue_t {
  struct queue_node_t *first, *last;

  pthread_mutex_t mutex;
  pthread_cond_t cond;

  enum queue_state state;
} queue_t;

/**
 * @brief Initialize the queue. No other operation is permitted to be executed
 * on given queue before queue_init() finishes.
 *
 * @param queue Newly created queue.
 * @return int 0 if queue is successfully initialized and can be used.
 * EINVAL if queue is NULL.
 * Error code from pthread_mutex_init or pthread_cond_init if either fails.
 * If the return value is not 0, queue is in an invalid state.
 */
int queue_init(queue_t *queue);

/**
 * @brief Add data to the tail of the queue.
 *
 * @param queue Successfully initialized queue.
 * @param data Data to be added. Data should either be heap-allocated,
 * or it should be ensured that it will not be present in the queue when
 * queue_destroy() is called.
 * @return int 0 if data was added correctly.
 * EINVAL if queue is NULL or in an invalid state.
 * ENOMEM if new node could not be created.
 * Error code from pthread_mutex_lock, pthread_mutex_unlock
 * or pthread_cond_signal if any of those fail.
 * If the return value is not 0, queue is in an invalid state.
 */
int queue_push(queue_t *queue, void *data);

/**
 * @brief Take data from the head of the queue. If the queue is empty,
 * function will block until data is available.
 *
 * @param queue Successfully initialzed queue.
 * @param data Location of returned data. Cannot be NULL.
 * *data may be NULL after calling this function if queue_destroy()
 * was called before data was available or timeout was reached.
 * @param timeout Timeout in seconds. If data are not available
 * before timeout is reached, function will return with *data set to NULL.
 * If timeout is 0, function will block until data are available.
 * @return int 0 if data was retrieved correctly.
 * EINVAL if queue is NULL, data are NULL or queue is in an invalid state.
 * Error code from pthread_mutex_lock, pthread_cond_wait or
 * pthread_mutex_unlock if any of those fail.
 * If the return value is not 0, queue is in an invalid state.
 */
int queue_pop(queue_t *queue, void **data, time_t timeout);

/**
 * @brief Release memory owned by queue and make it unusable.
 * If any threads are waiting on queue_pop(), they will be unblocked.
 *
 * @param queue Successfully initialized queue.
 * @return int 0 if queue was destroyed correctly.
 * EINVAL if queue is NULL or already destroyed.
 * Error code from pthread_cond_signal, pthread_mutex_lock,
 * pthread_mutex_unlock, pthread_mutex_destroy or pthread_cond_destroy
 * if any of those fail.
 */
int queue_destroy(queue_t *queue);

#endif // CPU_USAGE_TRACKER_QUEUE_H