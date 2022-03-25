#ifndef CPU_USAGE_TRACKER_PRINTER_H
#define CPU_USAGE_TRACKER_PRINTER_H

#include "common.h"

typedef struct printer_params_t {
  queue_t *analyzer_queue;
  queue_t *watchdog_queue;
  queue_t *logger_queue;
  FILE *output_file;
  volatile sig_atomic_t *exit_flag;
  int nprocs;
} printer_params_t;

void *printer_thread(void *printer_params);

#endif