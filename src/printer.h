#ifndef CPU_USAGE_TRACKER_PRINTER_H
#define CPU_USAGE_TRACKER_PRINTER_H

#include <stdio.h>

#include "common.h"

typedef struct printer_params_t {
  queue_t *analyzer_queue;
  queue_t *watchdog_queue;
  FILE *output_file;
  int *exit_flag;
  int nprocs;
} printer_params_t;

void *printer_thread(void *printer_params);

#endif