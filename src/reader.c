#include <stdlib.h>
#include <unistd.h>

#include "reader.h"

#define BUFFER_LENGTH 4096

void *reader_thread(void *reader_params) {
  reader_params_t *params = (reader_params_t *)reader_params;

  while (!*params->exit_flag) {
    if (!fseek(params->stream, 0, SEEK_SET)) {
      char *buffer = (char *)malloc(BUFFER_LENGTH);

      if (buffer) {
        rewind(params->stream);
        fread(buffer, sizeof(char), BUFFER_LENGTH, params->stream);

        queue_push(params->queue, buffer);
      } else {
        fprintf(stderr, "malloc failed!");
      }
    } else {
      fprintf(stderr, "fseek failed!");
    }

    sleep(1);
  }

  return NULL;
}