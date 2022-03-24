#ifndef CPU_USAGE_TRACKER_COMMON_H
#define CPU_USAGE_TRACKER_COMMON_H

#include <stdlib.h>
#include <string.h>

#include "queue.h"

#define TAG_READER "Reader"
#define TAG_ANALYZER "Analyzer"
#define TAG_PRINTER "Printer"
#define TAG_WATCHDOG "Watchdog"
#define TAG_LOGGER "Logger"
#define MESSAGE_LENGTH 9

static inline void notify_watchdog(queue_t *queue, const char *tag) {
  char *msg = malloc(sizeof(char) * MESSAGE_LENGTH);
  strcpy(msg, tag);

  queue_push(queue, msg);
}

#endif // CPU_USAGE_TRACKER_COMMON_H