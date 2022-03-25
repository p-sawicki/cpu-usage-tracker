#ifndef CPU_USAGE_TRACKER_LOGGER_H
#define CPU_USAGE_TRACKER_LOGGER_H

#include "common.h"

typedef struct logger_params_t {
  queue_t *input_queue, *watchdog_queue;
  volatile sig_atomic_t *exit_flag;
  FILE *output_file;
} logger_params_t;

void *logger_thread(void *logger_params);

#endif // CPU_USAGE_TRACKER_LOGGER_H