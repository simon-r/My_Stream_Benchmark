
#include <math.h>
#include <omp.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *find_command_line_arg_value(const int argc, const char *argv[],
                                        const char *arg) {

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], arg) == 0) {
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        return argv[i + 1]; // Return pointer to the value
      } else {
        return NULL; // No value or next argument is a command
      }
    }
  }
  return NULL; // Argument not found
}

const int find_command_line_arg_value_v2(const int argc, const char *argv[],
                                         const char *arg) {

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], arg) == 0) {
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        return i + 1; // Return pointer to the value
      } else {
        return -1; // No value or next argument is a command
      }
    }
  }
  return -1; // Argument not found
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
 * Generates a random number using the given seed.
 *
 * @param seed The seed value for generating the random number.
 * @return The generated random number.
 */
unsigned int generate_random_number(unsigned int seed) {
  const unsigned int magic = 214013;
  const unsigned int m = 1 << 31;
  const unsigned int a = 16807;

  return (seed * a + magic) % m;
}

/**
 * Calculates the elapsed time between two timespec structures.
 *
 * @param start The starting timespec structure.
 * @param end The ending timespec structure.
 * @return The elapsed time in milliseconds.
 */
double get_time(struct timespec start, struct timespec end) {
  double elapsed = (double)(end.tv_sec - start.tv_sec) *
                   (double)1000LL; // Convert seconds to milliseconds
  elapsed += (double)(end.tv_nsec - start.tv_nsec) /
             (double)1000000LL; // Convert nanoseconds to milliseconds

  return elapsed;
}

/**
 * Computes the bandwidth based on the given parameters.
 *
 * @param nr_cpu         The number of CPUs.
 * @param nr_streams     The number of streams.
 * @param batch_vec_size The size of the batch vector.
 * @param average_time   The average time taken.
 * @param word_size      The size of each word.
 * @return               The computed bandwidth.
 */
double compute_bandwidth(const unsigned int nr_cpu,
                         const unsigned int nr_streams,
                         const size_t batch_vec_size, const double average_time,
                         const unsigned int word_size) {

  return (double)nr_streams * (double)batch_vec_size * (double)nr_cpu *
         (word_size) / (average_time / 1000.0);
}

/**
 * Computes the average of the given vector.
 *
 * @param v The vector.
 * @param n The size of the vector.
 * @return  The average of the vector.
 */
double average(const double *v, unsigned int n) {

  double sum = 0;

  for (unsigned int i = 0; i < n; i++) {
    sum += v[i];
  }

  return sum / (double)n;
}

/**
 * Computes the variance of the given vector.
 *
 * @param v The vector.
 * @param n The size of the vector.
 * @return  The variance of the vector.
 */
double variance(const double *v, unsigned int n) {

  double avg = average(v, n);
  double sum = 0;

  for (unsigned int i = 0; i < n; i++) {
    sum += (v[i] - avg) * (v[i] - avg);
  }
  return sum / (double)n;
}

/**
 * Computes the standard deviation of the given vector.
 *
 * @param v The vector.
 * @param n The size of the vector.
 * @return  The standard deviation of the vector.
 */
double std_dev(const double *v, unsigned int n) { return sqrt(variance(v, n)); }

double maximum(const double *v, unsigned int n) {
  double max = v[0];
  for (unsigned int i = 1; i < n; i++) {
    if (v[i] > max) {
      max = v[i];
    }
  }
  return max;
}

double minimum(const double *v, unsigned int n) {
  double min = v[0];
  for (unsigned int i = 1; i < n; i++) {
    if (v[i] < min) {
      min = v[i];
    }
  }
  return min;
}

/**
 * Prints the help message.
 */
void print_help(char *argv[]) {
  printf("Usage: %s [options]\n", argv[0]);
  printf("Options:\n");
  printf("  -h, --help                  Show this help message and exit.\n");
  printf("  -s SIZE                     Size of the vector.\n");
  printf("  -r REPETITIONS              Number of repetitions of each "
         "benchmark.\n");

  printf("\n");
  printf("Description:\n");
  printf("  This program,  \"my_stream\" "
         ", is designed to benchmark the memory bandwidth (in Mb/s and "
         "Gb/s). \n"
         "  In order to measure the bandwidth, it executes four "
         "\"memory bound\" vector operations: Axpy, Copy, FMA (fused "
         "multiply-add), and Add Mult..\n"
         "  Visit: https://github.com/simon-r/My_Stream_Benchmark \n");

  printf("\n");
}

void print_performance_metrics(double bandwidth_axpy, double avg_clock_axpy,
                               double bandwidth_fma, double avg_clock_fma,
                               double bandwidth_copy, double avg_clock_copy,
                               double bandwidth_addmul, double avg_clock_addmul,
                               double to_Gb) {
    printf("\n-----------------------------------------------------------\n");
    printf("Test          Bandwidth [Gb/s]     Avg. Clock [ms]\n");
    printf("-----------------------------------------------------------\n");
    printf("AXPY:         %5.3f [Gb/s]         %5.3f [ms]\n", bandwidth_axpy / to_Gb,
           avg_clock_axpy);
    printf("FMA:          %5.3f [Gb/s]         %5.3f [ms]\n", bandwidth_fma / to_Gb,
           avg_clock_fma);
    printf("COPY:         %5.3f [Gb/s]         %5.3f [ms]\n", bandwidth_copy / to_Gb,
           avg_clock_copy);
    printf("ADDMUL:       %5.3f [Gb/s]         %5.3f [ms]\n", bandwidth_addmul / to_Gb,
           avg_clock_addmul);
    printf("-----------------------------------------------------------\n\n");
}