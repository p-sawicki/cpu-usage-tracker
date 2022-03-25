#ifndef CPU_USAGE_TRACKER_ANALYZER_H
#define CPU_USAGE_TRACKER_ANALYZER_H

#include "common.h"

typedef struct analyzer_params_t {
  queue_t *in_queue;
  queue_t *out_queue;
  queue_t *watchdog_queue;
  queue_t *logger_queue;
  int nprocs;
} analyzer_params_t;

void *analyzer_thread(void *analyzer_params);

#endif // CPU_USAGE_TRACKER_ANALYZER_H