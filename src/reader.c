#include <unistd.h>

#include "reader.h"

#define BUFFER_LENGTH 4096

void *reader_thread(void *reader_params) {
  reader_params_t *params = (reader_params_t *)reader_params;

  while (0 == exit_flag) {
    if (0 != notify_watchdog(params->watchdog_queue, TAG_READER)) {
      break;
    }

    if (0 == fseek(params->stream, 0, SEEK_SET)) {
      char *buffer = (char *)malloc(BUFFER_LENGTH);

      if (NULL != buffer) {
        rewind(params->stream);
        if (0 == fread(buffer, sizeof(char), BUFFER_LENGTH, params->stream)) {
          log_to_file(params->logger_queue, TAG_READER, "fread() failed!\n");
        } else {
          if (0 == queue_push(params->analyzer_queue, buffer)) {
            log_to_file(params->logger_queue, TAG_READER, "Sent message\n");
          } else {
            log_to_file(params->logger_queue, TAG_READER,
                        "queue_push() failed!\n");
            free(buffer);
            break;
          }
        }
      } else {
        log_to_file(params->logger_queue, TAG_READER, "malloc() failed!\n");
      }
    } else {
      log_to_file(params->logger_queue, TAG_READER, "fseek() failed!\n");
    }

    sleep(1);
  }
  log_to_file(params->logger_queue, TAG_READER, "Terminating\n");

  return NULL;
}
