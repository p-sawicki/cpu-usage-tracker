#include <unistd.h>

#include "reader.h"

#define BUFFER_LENGTH 4096

void *reader_thread(void *reader_params) {
  reader_params_t *params = (reader_params_t *)reader_params;

  while (!*params->exit_flag) {
    notify_watchdog(params->watchdog_queue, TAG_READER);

    if (!fseek(params->stream, 0, SEEK_SET)) {
      char *buffer = (char *)malloc(BUFFER_LENGTH);

      if (buffer) {
        rewind(params->stream);
        fread(buffer, sizeof(char), BUFFER_LENGTH, params->stream);

        if (queue_push(params->analyzer_queue, buffer)) {
          log_to_file(params->logger_queue, TAG_READER,
                      "queue_push() failed!\n");
        } else {
          log_to_file(params->logger_queue, TAG_READER, "Sent message\n");
        }
      } else {
        log_to_file(params->logger_queue, TAG_READER, "malloc() failed!\n");
      }
    } else {
      log_to_file(params->logger_queue, TAG_READER, "fseek() failed!\n");
    }

    sleep(1);
  }

  return NULL;
}