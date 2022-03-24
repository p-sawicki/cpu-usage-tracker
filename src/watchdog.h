#ifndef CPU_USAGE_TRACKER_WATCHDOG_H
#define CPU_USAGE_TRACKER_WATCHDOG_H

#include "common.h"

typedef struct watchdog_params_t {
  queue_t *queue;
  int *exit_flag;
} watchdog_params_t;

void *watchdog_thread(void *watchdog_params);

#endif // CPU_USAGE_TRACKER_WATCHDOG_H