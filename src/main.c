#include <sys/sysinfo.h>

#include "analyzer.h"
#include "reader.h"

#define PROC_STAT "/proc/stat"

int main() {
  int exit_flag = 0;
  queue_t reader_analyzer_queue;
  queue_init(&reader_analyzer_queue);

  queue_t analyzer_writer_queue;
  queue_init(&analyzer_writer_queue);

  reader_params_t reader_params;
  reader_params.exit_flag = &exit_flag;
  reader_params.queue = &reader_analyzer_queue;
  reader_params.stream = fopen(PROC_STAT, "r");

  pthread_t reader;
  pthread_create(&reader, NULL, reader_thread, &reader_params);

  int nprocs = get_nprocs();
  analyzer_params_t analyzer_params;
  analyzer_params.exit_flag = &exit_flag;
  analyzer_params.nprocs = nprocs;
  analyzer_params.queue_in = &reader_analyzer_queue;
  analyzer_params.queue_out = &analyzer_writer_queue;

  pthread_t analyzer;
  pthread_create(&analyzer, NULL, analyzer_thread, &analyzer_params);

  pthread_join(reader, NULL);
  pthread_join(&analyzer, NULL);
}