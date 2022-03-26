#include "printer.h"

#define CORES_PER_LINE 5

static int print_time(FILE *output) {
  char buffer[TIME_BUFFER_LENGTH];

  if (0 == get_time(buffer)) {
    return fprintf(output, "%s\n", buffer) != TIME_BUFFER_LENGTH;
  }

  return 1;
}

void *printer_thread(void *printer_params) {
  printer_params_t *params = (printer_params_t *)printer_params;
  size_t core_idx, lines_to_print;
  double *usage;

  while (0 == exit_flag) {
    if (0 != notify_watchdog(params->watchdog_queue, TAG_PRINTER)) {
      break;
    }
    usage = NULL;

    if (0 == queue_pop(params->analyzer_queue, (void **)&usage, 0) &&
        NULL != usage) {
      log_to_file(params->logger_queue, TAG_PRINTER, "Received message\n");
      fprintf(params->output_file, "\n");

      if (0 == print_time(params->output_file)) {
        lines_to_print = params->nprocs / CORES_PER_LINE;
        if (0 != params->nprocs % CORES_PER_LINE) {
          ++lines_to_print;
        }

        core_idx = 0;
        for (size_t line = 0; line < lines_to_print; ++line) {
          for (size_t i = 0; i < CORES_PER_LINE && core_idx < params->nprocs;
               ++i) {
            fprintf(params->output_file, "#%02ld: %.2lf%%\t", core_idx,
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
      break;
    }
  }
  log_to_file(params->logger_queue, TAG_PRINTER, "Terminating\n");

  return NULL;
}
