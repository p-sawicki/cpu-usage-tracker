#ifndef CPU_USAGE_TRACKER_READER_H
#define CPU_USAGE_TRACKER_READER_H

#include <stdio.h>

#include "queue.h"

typedef struct reader_params_t {
  queue_t *queue;
  FILE *stream;
  int *exit_flag;
} reader_params_t;

void *reader_thread(void *reader_params);

#endif // CPU_USAGE_TRACKER_READER_H