#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer.h"

#define VALUES_PER_LINE 11
#define STAT_LINE_START "cpu"
#define CPU_ID_BEGIN_INDEX 3
typedef unsigned long long ull;

double *parse_contents(const char *stat, int nprocs, ull *prev_total,
                       ull *prev_idle) {
  char *begin = strstr(stat, STAT_LINE_START);
  if (!begin) {
    fprintf(stderr, "[Analyzer] Invalid file contents received!\n");
    return NULL;
  }

  double *usage = malloc(sizeof(double) * nprocs);
  const char *line = strtok(begin, "\n");

  // Check that line begins with "cpu".
  while (strstr(line, STAT_LINE_START) == line) {
    // First line contains total usage so it should be skipped.
    // It starts with "cpu " while lines with data for individual cores
    // start with "cpuN" so isdigit() returns true only for those.
    if (isdigit(line[CPU_ID_BEGIN_INDEX])) {
      int cpu_index;
      ull user, nice, system, idle, iowait, irq, softirq, steal, guest,
          guest_nice;

      if (sscanf(line, "cpu%d%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld",
                 &cpu_index, &user, &nice, &system, &idle, &iowait, &irq,
                 &softirq, &steal, &guest, &guest_nice) == VALUES_PER_LINE) {
        if (cpu_index < 0 || cpu_index >= nprocs) {
          fprintf(stderr, "[Analyzer] Invalid CPU ID!\n");
          continue;
        }

        ull idle = idle + iowait;
        ull non_idle = user + nice + system + irq + softirq + steal;
        ull total = idle + non_idle;

        ull total_diff = total - prev_total[cpu_index];
        ull idle_diff = idle - prev_idle[cpu_index];
        usage[cpu_index] = total_diff == 0
                               ? 0.0
                               : (double)(total_diff - idle_diff) / total_diff;

        prev_total[cpu_index] = total;
        prev_idle[cpu_index] = idle;
      } else {
        fprintf(stderr, "[Analyzer] Invalid line format!\n");
      }
    }

    line = strtok(NULL, "\n");
  }

  return usage;
}

void *analyzer_thread(void *analyzer_params) {
  analyzer_params_t *params = (analyzer_params_t *)analyzer_params;

  ull *prev_total = calloc(params->nprocs, sizeof(ull));
  ull *prev_idle = calloc(params->nprocs, sizeof(ull));

  while (!*params->exit_flag) {
    char *stat = NULL;
    if (!queue_pop(params->queue_in, (void **)&stat, 0) && stat) {
      double *usage =
          parse_contents(stat, params->nprocs, prev_total, prev_idle);

      if (usage) {
        if (queue_push(params->queue_out, usage)) {
          fprintf(stderr, "[Analyzer] queue_push failed!");
        }
      }

      free(stat);
    } else {
      fprintf(stderr, "[Analyzer] queue_pop failed!");
    }
  }

  free(prev_idle);
  free(prev_total);

  return NULL;
}