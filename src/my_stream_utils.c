
#include <omp.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *find_command_line_arg_value(int argc, char *argv[], const char *arg) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], arg) == 0) {
      // Check if the next argument exists and is not another command
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        return argv[i + 1]; // Return pointer to the value
      } else {
        return NULL; // No value or next argument is a command
      }
    }
  }
  return NULL; // Argument not found
}

int flag_exists(int argc, char *argv[], const char *flag) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], flag) == 0) {
      return 1; // Return 1 if the flag is found
    }
  }
  return 0; // Return 0 if the flag is not found
}

int is_number(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] < '0' || str[i] > '9') {
      return 0;
    }
  }
  return 1;
}

/**
 * @brief
 *
 * @param seed
 * @return unsigned int
 */
unsigned int generate_random_number(unsigned int seed) {
  unsigned int magic = 214013;
  unsigned int m = 1 << 31;
  unsigned int a = 16807;

  return (seed * a + magic) % m;
}

double get_time(struct timespec start, struct timespec end) {
  double elapsed = (double)(end.tv_sec - start.tv_sec) *
                   (double)1000LL; // Convert seconds to milliseconds
  elapsed += (double)(end.tv_nsec - start.tv_nsec) /
             (double)1000000LL; // Convert nanoseconds to milliseconds

  return elapsed;
}

double compute_bandwidth(const unsigned int nr_cpu,      //
                         const unsigned int nr_streams,  //
                         const size_t batch_vec_size,    //
                         const double average_time,      //
                         const unsigned int word_size) { //

  return (double)nr_streams * (double)batch_vec_size * (double)nr_cpu *
          (word_size) /
         (average_time / 1000.0);
}