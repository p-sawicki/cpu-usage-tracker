#include "watchdog.h"

#define TIMEOUT_SECONDS 1
#define TIME_DIFF_TOLERANCE 2
#define LOG(tag)                                                               \
  log_to_file(params->logger_queue, TAG_WATCHDOG,                              \
              "Received notification from " tag "\n")

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
  while (0 == exit_flag) {
    if (0 == queue_pop(params->input_queue, (void **)&msg, TIMEOUT_SECONDS)) {
      time_now = time(NULL);

      if (NULL != msg) {
        if (0 == strcmp(msg, TAG_READER)) {
          time_reader = time_now;
          LOG(TAG_READER);
        } else if (0 == strcmp(msg, TAG_ANALYZER)) {
          time_analyzer = time_now;
          LOG(TAG_ANALYZER);
        } else if (0 == strcmp(msg, TAG_PRINTER)) {
          time_printer = time_now;
          LOG(TAG_PRINTER);
        } else if (0 == strcmp(msg, TAG_LOGGER)) {
          time_logger = time_now;
          LOG(TAG_LOGGER);
        }

        free(msg);
      }

      if (has_hanged(TAG_READER, time_now, time_reader) ||
          has_hanged(TAG_ANALYZER, time_now, time_analyzer) ||
          has_hanged(TAG_PRINTER, time_now, time_printer) ||
          has_hanged(TAG_LOGGER, time_now, time_logger)) {
        exit_flag = 1;
      }
    } else {
      log_to_file(params->logger_queue, TAG_WATCHDOG, "queue_pop() failed!\n");
      break;
    }
  }
  log_to_file(params->logger_queue, TAG_WATCHDOG, "Terminating\n");

  return NULL;
}