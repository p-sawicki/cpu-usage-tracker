#include "logger.h"

#define NOTIFICATION_DELAY 1

void *logger_thread(void *logger_params) {
  logger_params_t *params = (logger_params_t *)logger_params;
  time_t last_notified = 0;

  while (0 == *params->exit_flag) {
    time_t time_now = time(NULL);

    // Limit throughput of pings from logger to 1 / second so that
    // there is not too much unnecessary work.
    if (time_now - last_notified >= NOTIFICATION_DELAY) {
      notify_watchdog(params->watchdog_queue, TAG_LOGGER);
      last_notified = time_now;
    }

    char *msg = NULL;
    if (0 == queue_pop(params->input_queue, (void **)&msg, 0)) {
      if (NULL != msg) {
        char buffer[TIME_BUFFER_LENGTH];
        if (0 == get_time(buffer)) {
          fprintf(params->output_file, "[%s]%s", buffer, msg);
        } else {
          fprintf(stderr, "[Logger] get_time() failed!\n");
        }

        free(msg);
      }
    } else {
      fprintf(stderr, "[Logger] queue_pop() failed!\n");
    }
  }

  return NULL;
}