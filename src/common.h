#ifndef CPU_USAGE_TRACKER_COMMON_H
#define CPU_USAGE_TRACKER_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#define TAG_READER "Reader"
#define TAG_ANALYZER "Analyzer"
#define TAG_PRINTER "Printer"
#define TAG_WATCHDOG "Watchdog"
#define TAG_LOGGER "Logger"
#define MESSAGE_LENGTH 9
#define TIME_BUFFER_LENGTH 9

static inline void notify_watchdog(queue_t *queue, const char *tag) {
  char *msg = malloc(sizeof(char) * MESSAGE_LENGTH);
  strcpy(msg, tag);

  queue_push(queue, msg);
}

static inline int get_time(char *formatted_time) {
  time_t timer = time(NULL);
  struct tm *tm_info = localtime(&timer);

  if (NULL == tm_info ||
      TIME_BUFFER_LENGTH - 1 !=
          strftime(formatted_time, TIME_BUFFER_LENGTH, "%H:%M:%S", tm_info)) {
    return 1;
  }

  return 0;
}

static inline void log_to_file(queue_t *queue, const char *tag,
                               const char *msg) {
  size_t tag_length = strlen(tag);
  size_t formatted_length =             // +3 to enclose tag in brackets and add
      3 + tag_length + strlen(msg) + 1; // a space, +1 for terminating \0.

  char *buffer = malloc(sizeof(char) * formatted_length);

  if ((int)formatted_length - 1 != sprintf(buffer, "[%s] %s", tag, msg)) {
    fprintf(stderr, "sprintf() in log() failed!\n");
  }

  if (0 != queue_push(queue, buffer)) {
    fprintf(stderr, "queue_push() in log() failed!\n");
  }
}

#endif // CPU_USAGE_TRACKER_COMMON_H