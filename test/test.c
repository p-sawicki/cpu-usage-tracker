#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"

typedef struct consumer_thread_params_t {
  queue_t *queue;
  const char *expected_value;
  time_t timeout;
} consumer_thread_params_t;

static void *consumer_thread(void *thread_params) {
  consumer_thread_params_t *params = (consumer_thread_params_t *)thread_params;
  void *data = NULL;

  assert(0 == queue_pop(params->queue, &data, params->timeout));
  assert((params->expected_value == NULL && data == NULL) ||
         0 == strcmp(params->expected_value, (const char *)data));

  return NULL;
}

/**
 * @brief Testing the following scenario:
 * 1. Consumer1 starts and tries to take data.
 * 2. Elem1 is inserted - should be taken by Consumer1.
 * 3. Elem2 is inserted - should stay in queue.
 * 4. Consumer2 starts and should take Elem2.
 * Sleep()s were added to ensure correct order for testing.
 *
 */
static void queue_test() {
  queue_t queue;
  char *elem1 = "test1";
  char *elem2 = "test2";
  pthread_t consumer1, consumer2;
  consumer_thread_params_t params1, params2;
  assert(0 == queue_init(&queue));

  params1.expected_value = elem1;
  params1.queue = &queue;
  params1.timeout = 0;
  pthread_create(&consumer1, NULL, consumer_thread, (void *)&params1);
  sleep(1);

  assert(0 == queue_push(&queue, (void *)elem1));
  assert(0 == queue_push(&queue, (void *)elem2));
  sleep(1);

  params2.expected_value = elem2;
  params2.queue = &queue;
  params2.timeout = 0;
  pthread_create(&consumer2, NULL, consumer_thread, (void *)&params2);

  pthread_join(consumer1, NULL);
  pthread_join(consumer2, NULL);

  assert(0 == queue_destroy(&queue));
}

/**
 * @brief Tests that data left in queue are deallocated on queue_destroy().
 *
 */
static void queue_destroy_test() {
  queue_t queue;
  void *data = malloc(1024); // Should be freed in queue_destoy().
  assert(0 == queue_init(&queue));

  assert(0 == queue_push(&queue, data));
  assert(0 == queue_destroy(&queue));
  assert(NULL == queue.first);
}

/**
 * @brief Tests that all threads blocked in queue are unblocked in
 * queue_destroy().
 *
 */
static void queue_multiple_consumers_on_destroy_test() {
  queue_t queue;
  consumer_thread_params_t params;
  pthread_t consumer1, consumer2;
  assert(0 == queue_init(&queue));

  params.expected_value = NULL;
  params.queue = &queue;

  pthread_create(&consumer1, NULL, consumer_thread, (void *)&params);
  pthread_create(&consumer2, NULL, consumer_thread, (void *)&params);
  sleep(1);

  assert(0 == queue_destroy(&queue));
  assert(0 == pthread_join(consumer1, NULL));
  assert(0 == pthread_join(consumer2, NULL));
}

/**
 * @brief Tests the following scenario:
 * 1. Consumer1 calls pop with timeout = 1.
 * 2. Consumer2 calls pop with timeout = 0 (infinite).
 * 3. Data is push to the queue after 2 seconds.
 *
 * Consumer1 is expected to receive NULL due to timeout.
 * Consumer2 should take the inserted data.
 *
 */
static void queue_timeout_test() {
  queue_t queue;
  pthread_t consumer1, consumer2;
  consumer_thread_params_t params1;
  consumer_thread_params_t params2;
  assert(0 == queue_init(&queue));

  params1.expected_value = NULL;
  params1.queue = &queue;
  params1.timeout = 1;

  params2.expected_value = "test";
  params2.queue = &queue;
  params2.timeout = 0;

  pthread_create(&consumer1, NULL, consumer_thread, &params1);
  pthread_create(&consumer2, NULL, consumer_thread, &params2);
  sleep(2);

  assert(0 == queue_push(&queue, (void *)"test"));
  pthread_join(consumer1, NULL);
  pthread_join(consumer2, NULL);
  assert(0 == queue_destroy(&queue));
}

int main() {
  queue_test();
  queue_destroy_test();
  queue_multiple_consumers_on_destroy_test();
  queue_timeout_test();

  return 0;
}
