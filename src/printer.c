#include "printer.h"

#define CORES_PER_LINE 5

int print_time(FILE *output) {
  char buffer[TIME_BUFFER_LENGTH];

  if (0 == get_time(buffer)) {
    return fprintf(output, "%s\n", buffer) != TIME_BUFFER_LENGTH;
  }

  return 1;
}

void *printer_thread(void *printer_params) {
  printer_params_t *params = (printer_params_t *)printer_params;

  while (0 == *params->exit_flag) {
    notify_watchdog(params->watchdog_queue, TAG_PRINTER);
    double *usage = NULL;

    if (0 == queue_pop(params->analyzer_queue, (void **)&usage, 0) &&
        NULL != usage) {
      log_to_file(params->logger_queue, TAG_PRINTER, "Received message\n");
      fprintf(params->output_file, "\n");

      if (0 == print_time(params->output_file)) {
        int lines_to_print = params->nprocs / CORES_PER_LINE;
        if (0 != params->nprocs % CORES_PER_LINE) {
          ++lines_to_print;
        }

        int core_idx = 0;
        for (int line = 0; line < lines_to_print; ++line) {
          for (int i = 0; i < CORES_PER_LINE && core_idx < params->nprocs;
               ++i) {
            fprintf(params->output_file, "#%02d: %.2lf%%\t", core_idx,
                    usage[core_idx] * 100.0);
            ++core_idx;
          }
          fprintf(params->output_file, "\n");
        }

      } else {
        log_to_file(params->logger_queue, TAG_PRINTER,
                    "print_time() failed!\n");
      }

      free(usage);
    } else {
      log_to_file(params->logger_queue, TAG_PRINTER, "queue_pop() failed!\n");
    }
  }

  return NULL;
}