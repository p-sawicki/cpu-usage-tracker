#include <ctype.h>

#include "analyzer.h"

#define VALUES_PER_LINE 11
#define STAT_LINE_START "cpu"
#define CPU_ID_BEGIN_INDEX 3
typedef unsigned long long ull;

static double *parse_contents(const char *stat, size_t nprocs, ull *prev_total,
                              ull *prev_idle, queue_t *logger_queue) {
  char *begin = strstr(stat, STAT_LINE_START);
  double *usage;
  size_t cpu_index;
  ull user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  ull total_idle, non_idle, total, total_diff, idle_diff;
  const char *line;

  if (NULL == begin) {
    log_to_file(logger_queue, TAG_ANALYZER,
                "Invalid file contents received!\n");
    return NULL;
  }

  usage = malloc(sizeof(double) * (size_t)nprocs);
  if (NULL == usage) {
    log_to_file(logger_queue, TAG_ANALYZER, "malloc() failed!\n");
    return NULL;
  }

  line = strtok(begin, "\n");

  // Check that line begins with "cpu".
  while (strstr(line, STAT_LINE_START) == line) {
    // First line contains total usage so it should be skipped.
    // It starts with "cpu " while lines with data for individual cores
    // start with "cpuN" so isdigit() returns true only for those.
    if (isdigit(line[CPU_ID_BEGIN_INDEX])) {
      if (sscanf(line, "cpu%ld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld",
                 &cpu_index, &user, &nice, &system, &idle, &iowait, &irq,
                 &softirq, &steal, &guest, &guest_nice) == VALUES_PER_LINE) {
        if (cpu_index >= nprocs) {
          log_to_file(logger_queue, TAG_ANALYZER, "Invalid CPU ID!\n");
          continue;
        }
        // Calculation based on formula from
        // https://stackoverflow.com/a/23376195/13162312

        total_idle = idle + iowait;
        non_idle = user + nice + system + irq + softirq + steal;
        total = total_idle + non_idle;

        total_diff = total - prev_total[cpu_index];
        idle_diff = total_idle - prev_idle[cpu_index];
        usage[cpu_index] = total_diff == 0 ? 0.0
                                           : (double)(total_diff - idle_diff) /
                                                 (double)total_diff;

        prev_total[cpu_index] = total;
        prev_idle[cpu_index] = total_idle;
      } else {
        log_to_file(logger_queue, TAG_ANALYZER, "Invalid line format!\n");
      }
    }

    line = strtok(NULL, "\n");
  }

  return usage;
}

void *analyzer_thread(void *analyzer_params) {
  analyzer_params_t *params = (analyzer_params_t *)analyzer_params;
  char *stat;
  double *usage;

  ull *prev_total = calloc(params->nprocs, sizeof(ull));
  ull *prev_idle = calloc(params->nprocs, sizeof(ull));
  if (NULL == prev_total || NULL == prev_idle) {
    log_to_file(params->logger_queue, TAG_ANALYZER, "calloc() failed!\n");

    if (NULL != prev_total) {
      free(prev_total);
    }
    if (NULL != prev_idle) {
      free(prev_idle);
    }
    return NULL;
  }

  while (0 == exit_flag) {
    if (0 != notify_watchdog(params->watchdog_queue, TAG_ANALYZER)) {
      break;
    }

    stat = NULL;
    if (0 == queue_pop(params->in_queue, (void **)&stat, 0) && NULL != stat) {
      log_to_file(params->logger_queue, TAG_ANALYZER, "Received message\n");

      usage = parse_contents(stat, params->nprocs, prev_total, prev_idle,
                             params->logger_queue);

      if (NULL != usage) {
        if (0 == queue_push(params->out_queue, usage)) {
          log_to_file(params->logger_queue, TAG_ANALYZER, "Sent message\n");
        } else {
          log_to_file(params->logger_queue, TAG_ANALYZER,
                      "queue_push() failed!\n");

          free(usage);
          break;
        }
      }

      free(stat);
    } else {
      log_to_file(params->logger_queue, TAG_ANALYZER, "queue_pop() failed!\n");
      break;
    }
  }

  free(prev_idle);
  free(prev_total);
  log_to_file(params->logger_queue, TAG_ANALYZER, "Terminating\n");

  return NULL;
}
