#include <stdio.h>

#include "watchdog.h"

#define TIMEOUT_SECONDS 1
#define TIME_DIFF_TOLERANCE 2

int has_hanged(const char *tag, time_t time_now, time_t time_last) {
  if (time_now - time_last >= TIME_DIFF_TOLERANCE) {
    fprintf(stderr, "%s thread has hanged! Quitting...\n", tag);
    return 1;
  }

  return 0;
}

void *watchdog_thread(void *watchdog_params) {
  watchdog_params_t *params = (watchdog_params_t *)watchdog_params;

  time_t time_now = time(NULL), time_reader, time_analyzer, time_printer,
         time_logger;
  time_reader = time_analyzer = time_printer = time_logger = time_now;

  char *msg = NULL;
  while (0 == *params->exit_flag) {
    if (0 == queue_pop(params->queue, (void **)&msg, TIMEOUT_SECONDS)) {
      time_now = time(NULL);

      if (NULL != msg) {
        if (0 == strcmp(msg, TAG_READER)) {
          time_reader = time_now;
        } else if (0 == strcmp(msg, TAG_ANALYZER)) {
          time_analyzer = time_now;
        } else if (0 == strcmp(msg, TAG_PRINTER)) {
          time_printer = time_now;
        } else if (0 == strcmp(msg, TAG_LOGGER)) {
          time_logger = time_now;
        }

        free(msg);
      }

      if (has_hanged(TAG_READER, time_now, time_reader) ||
          has_hanged(TAG_ANALYZER, time_now, time_analyzer) ||
          has_hanged(TAG_PRINTER, time_now, time_printer) ||
          has_hanged(TAG_LOGGER, time_now, time_logger)) {
        *params->exit_flag = 1;
      }
    } else {
      fprintf(stderr, "[Watchdog] queue_pop() failed!\n");
    }
  }

  return NULL;
}