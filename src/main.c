#include <sys/sysinfo.h>

#include "analyzer.h"
#include "logger.h"
#include "printer.h"
#include "reader.h"
#include "watchdog.h"

#define PROC_STAT "/proc/stat"
#define LOG_FILE "/tmp/log.txt"

int main() {
  int exit_flag = 0;
  queue_t reader_analyzer_queue;
  queue_init(&reader_analyzer_queue);

  queue_t analyzer_printer_queue;
  queue_init(&analyzer_printer_queue);

  queue_t watchdog_queue;
  queue_init(&watchdog_queue);

  queue_t logger_queue;
  queue_init(&logger_queue);

  reader_params_t reader_params;
  reader_params.exit_flag = &exit_flag;
  reader_params.analyzer_queue = &reader_analyzer_queue;
  reader_params.stream = fopen(PROC_STAT, "r");
  reader_params.watchdog_queue = &watchdog_queue;
  reader_params.logger_queue = &logger_queue;

  pthread_t reader;
  pthread_create(&reader, NULL, reader_thread, &reader_params);

  int nprocs = get_nprocs();
  analyzer_params_t analyzer_params;
  analyzer_params.exit_flag = &exit_flag;
  analyzer_params.nprocs = nprocs;
  analyzer_params.in_queue = &reader_analyzer_queue;
  analyzer_params.out_queue = &analyzer_printer_queue;
  analyzer_params.watchdog_queue = &watchdog_queue;
  analyzer_params.logger_queue = &logger_queue;

  pthread_t analyzer;
  pthread_create(&analyzer, NULL, analyzer_thread, &analyzer_params);

  printer_params_t printer_params;
  printer_params.exit_flag = &exit_flag;
  printer_params.nprocs = nprocs;
  printer_params.output_file = stdout;
  printer_params.analyzer_queue = &analyzer_printer_queue;
  printer_params.watchdog_queue = &watchdog_queue;
  printer_params.logger_queue = &logger_queue;

  pthread_t printer;
  pthread_create(&printer, NULL, printer_thread, &printer_params);

  watchdog_params_t watchdog_params;
  watchdog_params.input_queue = &watchdog_queue;
  watchdog_params.exit_flag = &exit_flag;
  watchdog_params.logger_queue = &logger_queue;

  pthread_t watchdog;
  pthread_create(&watchdog, NULL, watchdog_thread, &watchdog_params);

  logger_params_t logger_params;
  logger_params.exit_flag = &exit_flag;
  logger_params.input_queue = &logger_queue;
  logger_params.output_file = fopen(LOG_FILE, "w");
  logger_params.watchdog_queue = &watchdog_queue;

  pthread_t logger;
  pthread_create(&logger, NULL, logger_thread, &logger_params);

  pthread_join(watchdog, NULL);

  queue_destroy(&reader_analyzer_queue);
  queue_destroy(&analyzer_printer_queue);
  queue_destroy(&watchdog_queue);

  pthread_join(reader, NULL);
  pthread_join(analyzer, NULL);
  pthread_join(printer, NULL);
  pthread_join(logger, NULL);

  fclose(reader_params.stream);
  fclose(logger_params.output_file);
}