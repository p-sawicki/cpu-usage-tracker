#include "logger.h"

#define NOTIFICATION_DELAY 1

void write_to_file(FILE *output_file, const char *msg) {
  char time_buffer[TIME_BUFFER_LENGTH];
  if (0 != get_time(time_buffer)) {
    fprintf(output_file, "%s", "[Logger] get_time() failed!\n");
    strcpy(time_buffer, "-");
  }

  fprintf(output_file, "[%s]%s", time_buffer, msg);
}

void *logger_thread(void *logger_params) {
  logger_params_t *params = (logger_params_t *)logger_params;
  time_t last_notified = 0;

  while (0 == exit_flag) {
    time_t time_now = time(NULL);

    // Limit throughput of pings from logger to 1 / second so that
    // there is not too much unnecessary work.
    if (time_now - last_notified >= NOTIFICATION_DELAY) {
      if (0 != notify_watchdog(params->watchdog_queue, TAG_LOGGER)) {
        break;
      }

      last_notified = time_now;
    }

    char *msg = NULL;
    if (0 == queue_pop(params->input_queue, (void **)&msg, 0) && NULL != msg) {
      write_to_file(params->output_file, msg);

      free(msg);
    } else {
      write_to_file(params->output_file, "[Logger] queue_pop() failed!\n");
      break;
    }
  }
  write_to_file(params->output_file, "[Logger] Terminating\n");

  return NULL;
}