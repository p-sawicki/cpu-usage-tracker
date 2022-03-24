#ifndef CPU_USAGE_TRACKER_ANALYZER_H
#define CPU_USAGE_TRACKER_ANALYZER_H

#include "queue.h"

typedef struct analyzer_params_t {
  queue_t *queue_in;
  queue_t *queue_out;
  int *exit_flag;
  int nprocs;
} analyzer_params_t;

void *analyzer_thread(void *analyzer_params);

#endif // CPU_USAGE_TRACKER_ANALYZER_H