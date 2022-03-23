#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"

typedef struct consumer_thread_params_t {
  queue_t *queue;
  const char *expected_value;
} consumer_thread_params_t;

void *consumer_thread(void *thread_params) {
  consumer_thread_params_t *params = (consumer_thread_params_t *)thread_params;
  void *data = NULL;
  int res;

  if ((res = queue_pop(params->queue, &data))) {
    return (void *)1l;
  }

  return (void *)(long)strcmp(params->expected_value, (const char *)data);
}

int queue_test() {
  queue_t queue;
  int res = 0;

  pthread_t consumer1, consumer2;

  const char *elem1 = "test1";
  const char *elem2 = "test2";

  if ((res = queue_init(&queue))) {
    return res;
  }

  /**
   * Testing the following scenario:
   * 1. Consumer1 starts and tries to take data.
   * 2. Elem1 is inserted - should be taken by Consumer1.
   * 3. Elem2 is inserted - should stay in queue.
   * 4. Consumer2 starts and should take Elem2.
   * Sleep()s were added to ensure correct order for testing.
   *
   */

  consumer_thread_params_t params1;
  params1.expected_value = elem1;
  params1.queue = &queue;
  pthread_create(&consumer1, NULL, consumer_thread, (void *)&params1);
  sleep(1);

  if ((res = queue_push(&queue, (void *)elem1))) {
    return res;
  }

  if ((res = queue_push(&queue, (void *)elem2))) {
    return res;
  }
  sleep(1);

  consumer_thread_params_t params2;
  params2.expected_value = elem2;
  params2.queue = &queue;
  pthread_create(&consumer2, NULL, consumer_thread, (void *)&params2);

  void *return_val = NULL;
  pthread_join(consumer1, &return_val);
  if ((long)return_val) {
    return 1;
  }

  pthread_join(consumer2, &return_val);
  if ((long)return_val) {
    return 1;
  }

  if ((res = queue_destroy(&queue))) {
    return res;
  }

  return 0;
}

int queue_destroy_test() {
  queue_t queue;
  int res;

  if ((res = queue_init(&queue))) {
    return res;
  }

  void *data = malloc(1024); // Should be freed in queue_destoy().
  if ((res = queue_push(&queue, data))) {
    return res;
  }

  if ((res = queue_destroy(&queue))) {
    return res;
  }

  return 0;
}

int main() {
  int res;
  if ((res = queue_test())) {
    return res;
  }

  if ((res = queue_destroy_test())) {
    return res;
  }

  return 0;
}