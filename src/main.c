#include "reader.h"

#define PROC_STAT "/proc/stat"

int main() {
  int exit_flag = 0;
  queue_t reader_analyzer_queue;
  queue_init(&reader_analyzer_queue);

  reader_params_t reader_params;
  reader_params.exit_flag = &exit_flag;
  reader_params.queue = &reader_analyzer_queue;
  reader_params.stream = fopen(PROC_STAT, "r");

  pthread_t reader;
  pthread_create(&reader, NULL, reader_thread, &reader_params);

  pthread_join(reader, NULL);
}