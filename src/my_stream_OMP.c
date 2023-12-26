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

struct streams_args {
  size_t vec_size;

  float_type *a;
  float_type *b;
  float_type *c;
  float_type *d;

  double clock;
  double bandwidth;

  double consume_out;
  size_t benchmark_repetitions;
  size_t nr_cpu;
};

void *stream_calloc(size_t __alignment, size_t vector_len, size_t type_size) {
  return (void *)aligned_alloc(__alignment, vector_len * type_size);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void fma_omp(struct streams_args *args) {

  const unsigned int vec_size = args->vec_size;

  float_type *a = args->a;
  float_type *b = args->b;
  float_type *c = args->c;
  float_type *d = args->d;

  float_type consume_out = 0.0;
  double elapsed = 0.0;

  struct timespec start, end;

  for (int r = 0; r < args->benchmark_repetitions; r++) {

    clock_gettime(CLOCK_MONOTONIC, &start);
#pragma omp parallel for
    for (size_t i = 0; i < vec_size; i++) {
      d[i] = a[i] * b[i] + c[i];
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed += get_time(start, end);

    consume_out += a[rand() % vec_size] + b[rand() % vec_size] +
                   c[rand() % vec_size] + d[rand() % vec_size];
    // printf("n %f ", consume_out);
  }

  args->consume_out = consume_out;
  args->clock = elapsed / (double)args->benchmark_repetitions;

  args->bandwidth = compute_bandwidth(1.0, 4.0, args->vec_size, args->clock,
                                      sizeof(float_type));
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void axpy_omp(struct streams_args *args) {

  const unsigned int vec_size = args->vec_size;

  float_type alpha = 2.56;

  float_type *a = args->a;
  float_type *b = args->b;
  float_type *c = args->c;
  float_type *d = args->d;

  float_type consume_out = 0.0;
  double elapsed = 0.0;

  struct timespec start, end;

  for (int r = 0; r < args->benchmark_repetitions; r++) {

    clock_gettime(CLOCK_MONOTONIC, &start);
#pragma omp parallel for
    for (size_t i = 0; i < vec_size; i++) {
      d[i] = alpha * a[i] + c[i];
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed += get_time(start, end);

    consume_out += a[rand() % vec_size] + b[rand() % vec_size] +
                   c[rand() % vec_size] + d[rand() % vec_size];
    // printf("n %f ", consume_out);
  }

  args->consume_out = consume_out;
  args->clock = elapsed / (double)args->benchmark_repetitions;

  args->bandwidth = compute_bandwidth(1.0, 3.0, args->vec_size, args->clock,
                                      sizeof(float_type));
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void copy_omp(struct streams_args *args) {

  const unsigned int vec_size = args->vec_size;

  float_type *a = args->a;
  float_type *d = args->d;

  float_type consume_out = 0.0;
  double elapsed = 0.0;

  struct timespec start, end;

  for (int r = 0; r < args->benchmark_repetitions; r++) {

    clock_gettime(CLOCK_MONOTONIC, &start);
#pragma omp parallel for
    for (size_t i = 0; i < vec_size; i++) {
      d[i] = a[i];
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed += get_time(start, end);

    consume_out += a[rand() % vec_size] + d[rand() % vec_size];
  }

  args->consume_out = consume_out;
  args->clock = elapsed / (double)args->benchmark_repetitions;

  args->bandwidth = compute_bandwidth(1.0, 2.0, args->vec_size, args->clock,
                                      sizeof(float_type));
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void addmul_omp(struct streams_args *args) {

  const unsigned int vec_size = args->vec_size;

  float_type *a = args->a;
  float_type *b = args->b;
  float_type *c = args->c;
  float_type *d = args->d;

  float_type consume_out = 0.0;
  double elapsed = 0.0;

  struct timespec start, end;

  for (int r = 0; r < args->benchmark_repetitions; r++) {

    clock_gettime(CLOCK_MONOTONIC, &start);
#pragma omp parallel for
    for (size_t i = 0; i < vec_size; i++) {
      d[i] = a[i] + b[i];
      c[i] = a[i] * b[i];
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed += get_time(start, end);

    consume_out += a[rand() % vec_size] + b[rand() % vec_size] +
                   c[rand() % vec_size] + d[rand() % vec_size];
    // printf("n %f ", consume_out);
  }

  args->consume_out = consume_out;
  args->clock = elapsed / (double)args->benchmark_repetitions;

  args->bandwidth = compute_bandwidth(1.0, 4.0, args->vec_size, args->clock,
                                      sizeof(float_type));
}

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
  printf("Adjusted vector size:      %lu\n", vec_size);
  printf("Mb Vector size:            %f [Mb]\n", Mb_vec_size);
  printf("Gb Vector size:            %f [Gb]\n", Gb_vec_size);
  printf("Gb Total allocated memory: %f [Gb]\n", Gb_vec_size * 4);
  printf("Repetitions:               %d\n", benchmark_repetitions);
  printf("-----------------------------------------------------------\n\n");

  struct streams_args args;

  args.vec_size = vec_size;
  args.a = (float_type *)stream_calloc(64, vec_size, sizeof(float_type));
  args.b = (float_type *)stream_calloc(64, vec_size, sizeof(float_type));
  args.c = (float_type *)stream_calloc(64, vec_size, sizeof(float_type));
  args.d = (float_type *)stream_calloc(64, vec_size, sizeof(float_type));

  unsigned int r = 1;
  for (int i = 0; i < vec_size; i++) {
    r = generate_random_number(r);
    args.a[i] = 1.0 + (float_type)(r % 300) / 200.0;
    args.b[i] = 1.0 + (float_type)(r % 400) / 300.0;
    args.c[i] = 1.0 + (float_type)(r % 500) / 300.0;
    args.d[i] = 0.0;
  }

  args.consume_out = 22.2;
  args.benchmark_repetitions = benchmark_repetitions;

  fma_omp(&args);

  printf("-----------------------------------------------------------\n");
  printf("FMA:                       %f [Gb/s]\n", args.bandwidth / to_Gb);
  printf("Clock:                     %f [ms]\n", args.clock);
  printf("Consume:  %lf\n", args.consume_out);

  args.consume_out = 22.2;
  args.benchmark_repetitions = benchmark_repetitions;

  axpy_omp(&args);

  printf("-----------------------------------------------------------\n");
  printf("AXPY:                      %f [Gb/s]\n", args.bandwidth / to_Gb);
  printf("Clock:                     %f [ms]\n", args.clock);
  printf("Consume:  %lf\n", args.consume_out);

  args.consume_out = 22.2;
  args.benchmark_repetitions = benchmark_repetitions;
  copy_omp(&args);

  printf("-----------------------------------------------------------\n");
  printf("Copy:                      %f [Gb/s]\n", args.bandwidth / to_Gb);
  printf("Clock:                     %f [ms]\n", args.clock);
  printf("Consume:  %lf\n", args.consume_out);

  args.consume_out = 22.2;
  args.benchmark_repetitions = benchmark_repetitions;
  addmul_omp(&args);
  
  printf("-----------------------------------------------------------\n");
  printf("Add Mul:                   %f [Gb/s]\n", args.bandwidth / to_Gb);
  printf("Clock:                     %f [ms]\n", args.clock);
  printf("Consume:  %lf\n", args.consume_out);
  printf("-----------------------------------------------------------\n");
  return 0;
}