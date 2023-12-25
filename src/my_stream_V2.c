/**
my_stream V2
Copyright (C) 2023

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file my_stream.c
 * @author Simone Riva (you@domain.com)
 * @brief
 * @version 0.0.1
 * @date 2023-12-16
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <omp.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "my_stream_utils.h"

#define DEFAULT_TEST_SIZE 50000000

#define VECTOR_LEN 8

#define BENCHMARK_REPETITIONS 50

#define VERBOSE
#undef VERBOSE

typedef double float_type;

typedef float_type vector_type
    __attribute__((vector_size(VECTOR_LEN * sizeof(float_type)), //
                   aligned(sizeof(float_type))));                //

struct streams_args {
  size_t size;

  double clock;
  double bandwidth;

  double consume_out;
  size_t benchmark_repetitions;

  sem_t *semaphore;
};

void *stream_calloc(size_t __alignment, size_t vector_len, size_t type_size) {
  return (void *)aligned_alloc(__alignment, vector_len * type_size);
}

/**
 * @brief
 *
 * @param arg_void
 * @return void*
 */
void *fma_thread(void *arg_void) {

  struct streams_args *args = (struct streams_args *)arg_void;

  size_t size = args->size;

  float_type *a = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              args->size, sizeof(float_type));
  float_type *b = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              args->size, sizeof(float_type));
  float_type *c = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              args->size, sizeof(float_type));
  float_type *d = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              args->size, sizeof(float_type));

  unsigned int r = 1;
  for (int i = 0; i < args->size; i++) {
    r = generate_random_number(r);
    a[i] = 1.0 + (float_type)(r % 300) / 200.0;
    b[i] = 1.0 + (float_type)(r % 400) / 300.0;
    d[i] = 0.0;
  }

  vector_type *a_vec = (vector_type *)(a);
  vector_type *b_vec = (vector_type *)(b);
  vector_type *d_vec = (vector_type *)(d);

  size_t size_vec = (args->size) / VECTOR_LEN;

  double alpha = 2.55;
  // start hi definition clock
  struct timespec start, end;
  double elapsed = 0.0;
  double consume_out = 0.0;

  for (int i = 0; i < args->benchmark_repetitions; i++) {
    sem_wait(args->semaphore);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < size_vec; i++) {
      d_vec[i] = alpha * a_vec[i] + b_vec[i];
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    sem_post(args->semaphore);

    elapsed += get_time(start, end);
    consume_out += a[rand() % size] + b[rand() % size] + c[rand() % size] +
                   d[rand() % size];
    alpha *= 1.01;
  }

  args->consume_out = consume_out;
  args->clock = elapsed / args->benchmark_repetitions;

  free(a);
  free(b);
  free(c);
  free(d);

  return NULL;
}

double execute_mt_fma_test(struct streams_args *th_args, int nr_cpu) {

  sem_t semaphore;
  sem_init(&semaphore, 0, nr_cpu);

  double avg_time = 0.0;
  double consume_out = 0.0;

  pthread_t *threads = malloc(nr_cpu * sizeof(pthread_t));

  for (int i = 0; i < nr_cpu; i++) {
    th_args[i].semaphore = &semaphore;
    pthread_create(&threads[i], NULL, fma_thread, &th_args[i]);
  }

  for (int i = 0; i < nr_cpu; i++) {
    pthread_join(threads[i], NULL);
    avg_time += th_args[i].clock;
    consume_out += th_args[i].consume_out;
  }
  avg_time /= nr_cpu;

  printf("FMA: %f ms\n", avg_time);
  printf("consume_out: %f\n", consume_out);

  const double bw = compute_bandwidth(nr_cpu, 3, th_args[0].size, avg_time, sizeof(float_type));

  printf("Bandwidth: %f Mb/s\n", bw / to_Mb);
  printf("Bandwidth: %f Gb/s\n", bw / to_Gb);

  free(threads);

  return avg_time;
}

void *copy_thread(void *arg_void) { return NULL; }

void *axpy_thread(void *arg_void) { return NULL; }

void *add_mult_thread(void *arg_void) { return NULL; }

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char **argv) {

  printf("Start My Stream\n------------------------\n\n");

  size_t vec_size = DEFAULT_TEST_SIZE;
  int benchmark_repetitions = BENCHMARK_REPETITIONS;

  if (flag_exists(argc, argv, "-h") | flag_exists(argc, argv, "--help")) {

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
    return 0;
  }

  char *vec_size_arg = find_command_line_arg_value(argc, argv, "-s");

  if (vec_size_arg != NULL) {

    if (is_number(vec_size_arg)) {
      vec_size = strtoul(vec_size_arg, NULL, 10);
      printf("User defined vector size: %lu\n", vec_size);
    } else {
      printf("Error: argv[1] is not numeric\n");
      return 1;
    }
  }

  char *benchmark_repetitions_arg =
      find_command_line_arg_value(argc, argv, "-r");

  if (benchmark_repetitions_arg != NULL) {
    if (is_number(benchmark_repetitions_arg)) {
      benchmark_repetitions = atoi(benchmark_repetitions_arg);
      printf("User defined benchmark repetitions: %d\n", benchmark_repetitions);
    } else {
      printf("Error: argv -r is not numeric\n");
      return 1;
    }
  }

  // get the number of cpu from open mp
  const int nr_cpu = omp_get_num_procs();
  vec_size = vec_size / nr_cpu;
  vec_size = ((vec_size - vec_size % VECTOR_LEN) + VECTOR_LEN) * nr_cpu;

  double bytes_vec_size = (double)(vec_size * sizeof(float_type));
  double Mb_vec_size = bytes_vec_size / to_Mb;
  double Gb_vec_size = bytes_vec_size / to_Gb;

  printf("\n-----------------------------------------------------------\n");
  printf("Number of CPU:             %d\n", nr_cpu);
  printf("Corrected vector size:     %lu\n", vec_size);
  printf("Mb Vector size:            %f [Mb]\n", Mb_vec_size);
  printf("Gb Vector size:            %f [Gb]\n", Gb_vec_size);
  printf("Gb Total allocated memory: %f [Gb]\n", Gb_vec_size * 4);
  printf("Repetitions:               %d\n", benchmark_repetitions);
  printf("-----------------------------------------------------------\n\n");

  struct streams_args *th_args = malloc(nr_cpu * sizeof(struct streams_args));
  size_t batch_vec_size = vec_size / nr_cpu;

  for (int i = 0; i < nr_cpu; i++) {
    th_args[i].size = batch_vec_size;
    th_args[i].benchmark_repetitions = benchmark_repetitions;
    th_args[i].consume_out = 0.0;
    th_args[i].clock = 0.0;
  }

  double avg_time = execute_mt_fma_test(th_args, nr_cpu);
  printf("Average time: %f ms\n", avg_time);

  return 0;
}