#include <stdlib.h>

#include "printer.h"

#define CORES_PER_LINE 5
#define TIME_BUFFER_LENGTH 9

int print_time(FILE *output) {
  time_t timer = time(NULL);
  struct tm *tm_info = localtime(&timer);

  if (NULL != tm_info) {
    char buffer[TIME_BUFFER_LENGTH];

    if (0 != strftime(buffer, TIME_BUFFER_LENGTH, "%H:%M:%S", tm_info)) {
      return fprintf(output, "%s\n", buffer) != TIME_BUFFER_LENGTH;
    }
  }

  return 1;
}

void *printer_thread(void *printer_params) {
  printer_params_t *params = (printer_params_t *)printer_params;

  while (0 == *params->exit_flag) {
    double *usage = NULL;
    if (0 == queue_pop(params->queue, (void **)&usage, 0) && NULL != usage) {
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
        fprintf(stderr, "[Printer] print_time() failed!\n");
      }

      free(usage);
    } else {
      fprintf(stderr, "[Printer] queue_pop failed!");
    }
  }

  return NULL;
}