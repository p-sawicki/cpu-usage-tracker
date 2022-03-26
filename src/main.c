#include <sys/sysinfo.h>

#include "analyzer.h"
#include "logger.h"
#include "printer.h"
#include "reader.h"
#include "watchdog.h"

#define PROC_STAT "/proc/stat"
#define LOG_FILE "/tmp/cut_log.txt"
#define CHECK_ERROR(EXP)                                                       \
  do {                                                                         \
    if (EXP && 0 == exit_flag)                                                 \
      error(#EXP);                                                             \
  } while (0)

// sa_handler and stdout generate warnings otherwise.
#ifdef __clang__
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif

volatile sig_atomic_t exit_flag = 0;

static void term(int signum) {
  (void)signum;
  exit_flag = 1;
}

static void error(const char *msg) {
  fprintf(stderr, "Fatal error: %s. Terminating...\n", msg);
  exit_flag = 1;
}

static void add_signal_handler() {
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = term;

  CHECK_ERROR(0 != sigaction(SIGTERM, &action, NULL));
  CHECK_ERROR(0 != sigaction(SIGINT, &action, NULL));
}

int main() {
  queue_t reader_analyzer_queue, analyzer_printer_queue, watchdog_queue,
      logger_queue;
  pthread_t reader, analyzer, printer, watchdog, logger;
  FILE *proc_stat = fopen(PROC_STAT, "r"), *output_file = fopen(LOG_FILE, "w");
  size_t nprocs = (size_t)get_nprocs();

  add_signal_handler();

  CHECK_ERROR(NULL == proc_stat);
  CHECK_ERROR(NULL == output_file);

  CHECK_ERROR(0 != queue_init(&reader_analyzer_queue));
  CHECK_ERROR(0 != queue_init(&analyzer_printer_queue));
  CHECK_ERROR(0 != queue_init(&watchdog_queue));
  CHECK_ERROR(0 != queue_init(&logger_queue));

  CHECK_ERROR(0 != pthread_create(&reader, NULL, reader_thread,
                                  &(reader_params_t){
                                      &reader_analyzer_queue, &watchdog_queue,
                                      &logger_queue, proc_stat}));
  CHECK_ERROR(0 != pthread_create(&analyzer, NULL, analyzer_thread,
                                  &(analyzer_params_t){&reader_analyzer_queue,
                                                       &analyzer_printer_queue,
                                                       &watchdog_queue,
                                                       &logger_queue, nprocs}));
  CHECK_ERROR(0 != pthread_create(&printer, NULL, printer_thread,
                                  &(printer_params_t){
                                      &analyzer_printer_queue, &watchdog_queue,
                                      &logger_queue, stdout, nprocs}));
  CHECK_ERROR(0 != pthread_create(
                       &watchdog, NULL, watchdog_thread,
                       &(watchdog_params_t){&watchdog_queue, &logger_queue}));
  CHECK_ERROR(0 !=
              pthread_create(&logger, NULL, logger_thread,
                             &(logger_params_t){&logger_queue, &watchdog_queue,
                                                output_file}));

  // Watchdog will be the first to finish because it never blocks indefinitely.
  // After this, exit_flag == 1.
  CHECK_ERROR(0 != pthread_join(watchdog, NULL));

  // Threads other than watchdog block on their input queues indefinitely.
  // Queues are destroyed before threads are joined to unblock them.

  CHECK_ERROR(0 != queue_destroy(&reader_analyzer_queue));
  CHECK_ERROR(0 != queue_destroy(&analyzer_printer_queue));
  CHECK_ERROR(0 != queue_destroy(&watchdog_queue));
  CHECK_ERROR(0 != queue_destroy(&logger_queue));

  CHECK_ERROR(0 != pthread_join(reader, NULL));
  CHECK_ERROR(0 != pthread_join(analyzer, NULL));
  CHECK_ERROR(0 != pthread_join(printer, NULL));
  CHECK_ERROR(0 != pthread_join(logger, NULL));

  CHECK_ERROR(0 != fclose(proc_stat));
  CHECK_ERROR(0 != fclose(output_file));

  printf("Goodbye.\n");
}
