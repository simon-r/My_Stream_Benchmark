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

#define GLOBAL_ALLOC __GLOBAL_ALLOC__

// #define OPENMP_VERSION 201810

typedef double float_type;

void *stream_calloc(size_t __alignment, size_t vector_len, size_t type_size) {

#if OPENMP_VERSION < 201811

#pragma message "Using alligned_alloc from C11"

  return (void *)aligned_alloc(__alignment, vector_len * type_size);
#else
#pragma message "Using omp_aligned_alloc from OpenMP 5.0"

  return (void *)omp_aligned_alloc(__alignment, vector_len * type_size,
                                   omp_get_default_allocator());
#endif
}

void stream_free(void *ptr) {

#if OPENMP_VERSION < 201811
  free(ptr);
#else
  omp_free(ptr, omp_get_default_allocator());
#endif
}

int main(int argc, char *argv[]) {

  size_t vec_size = DEFAULT_TEST_SIZE;
  int benchmark_repetitions = BENCHMARK_REPETITIONS;
  const int nr_cpu = omp_get_num_procs();

  printf("Start My Stream  [OpenMP]\n\n");

#ifdef COMPILER
  printf("Compiler: %s\n\n", COMPILER);
#endif

#ifdef ARCHITECTURE
  printf("Architecture: %s\n\n", ARCHITECTURE);
#endif

  if (flag_exists(argc, argv, "-h") | flag_exists(argc, argv, "--help")) {
    print_help(argv);
    return 0;
  }

  //   size_t test_size = DEFAULT_TEST_SIZE;

  char *vec_size_arg = find_command_line_arg_value(argc, argv, "-s");

  if (vec_size_arg != NULL) {

    if (is_number(vec_size_arg)) {
      vec_size = strtoul(vec_size_arg, NULL, 10);
      printf("User defined vector size: %lu\n", vec_size);
    } else {
      printf("Error: argument of -s is not numeric\n");
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

  vec_size = vec_size / nr_cpu;
  vec_size = ((vec_size - vec_size % VECTOR_LEN) + VECTOR_LEN) * nr_cpu;

  double bytes_vec_size = (double)(vec_size * sizeof(float_type));
  double MB_vec_size = bytes_vec_size / to_MB;
  double GB_vec_size = bytes_vec_size / to_GB;

  printf("\n-----------------------------------------------------------\n");
  printf("Number of CPU:             %d\n", nr_cpu);
  printf("Adjusted vector size:      %lu\n", vec_size);
  printf("MB Vector size:            %f [MB]\n", MB_vec_size);
  printf("GB Vector size:            %f [GB]\n", GB_vec_size);
  printf("GB Total allocated memory: %f [GB]\n", GB_vec_size * 4);
  printf("Repetitions:               %d\n", benchmark_repetitions);
  printf("-----------------------------------------------------------\n\n");

  double *clock_axpy = malloc(sizeof(float_type) * benchmark_repetitions);
  double *clock_fma = malloc(sizeof(float_type) * benchmark_repetitions);
  double *clock_copy = malloc(sizeof(float_type) * benchmark_repetitions);
  double *clock_addmul = malloc(sizeof(float_type) * benchmark_repetitions);

  { /// Begin benckmark
    float_type *a =
        (float_type *)stream_calloc(1024, vec_size, sizeof(float_type));
    float_type *b =
        (float_type *)stream_calloc(1024, vec_size, sizeof(float_type));
    float_type *c =
        (float_type *)stream_calloc(1024, vec_size, sizeof(float_type));
    float_type *d =
        (float_type *)stream_calloc(1024, vec_size, sizeof(float_type));

    double consume_out = 0.0;
    const double alpha = 2.56;

#pragma omp parallel for
    for (int i = 0; i < vec_size; i++) {
      a[i] = 1.0 + (float_type)(i % 300) / 200.0;
      b[i] = 1.0 + (float_type)(i % 200) / 150.0;
      c[i] = 1.0 + (float_type)(i % 150) / 100.0;
      d[i] = 0.0;
    }

    struct timespec start, end;

    //// FMA
    for (int r = 0; r < benchmark_repetitions; r++) {

      clock_gettime(CLOCK_MONOTONIC, &start);

#pragma omp parallel for
      for (size_t i = 0; i < vec_size; i++) {
        d[i] = a[i] * b[i] + c[i];
      }

      clock_gettime(CLOCK_MONOTONIC, &end);

      clock_fma[r] = get_time(start, end);

      consume_out += a[rand() % vec_size] + b[rand() % vec_size] +
                     c[rand() % vec_size] + d[rand() % vec_size];
      // printf("n %f ", consume_out);
    }

    //// AXPY
    for (int r = 0; r < benchmark_repetitions; r++) {

      clock_gettime(CLOCK_MONOTONIC, &start);

#pragma omp parallel for
      for (size_t i = 0; i < vec_size; i++) {
        d[i] = alpha * a[i] + b[i];
      }

      clock_gettime(CLOCK_MONOTONIC, &end);

      clock_axpy[r] = get_time(start, end);

      consume_out += d[rand() % vec_size];
      // printf("n %f ", consume_out);
    }

    //// COPY
    for (int r = 0; r < benchmark_repetitions; r++) {

      clock_gettime(CLOCK_MONOTONIC, &start);

#pragma omp parallel for
      for (size_t i = 0; i < vec_size; i++) {
        d[i] = a[i];
      }

      clock_gettime(CLOCK_MONOTONIC, &end);

      clock_copy[r] = get_time(start, end);

      consume_out += d[rand() % vec_size];
      // printf("n %f ", consume_out);
    }

    //// ADDMUL
    for (int r = 0; r < benchmark_repetitions; r++) {

      clock_gettime(CLOCK_MONOTONIC, &start);

#pragma omp parallel for
      for (size_t i = 0; i < vec_size; i++) {
        d[i] = a[i] + b[i];
        c[i] = a[i] * b[i];
      }

      clock_gettime(CLOCK_MONOTONIC, &end);

      clock_addmul[r] = get_time(start, end);

      consume_out += c[rand() % vec_size] + d[rand() % vec_size];
      // printf("n %f ", consume_out);
    }

    //    omp_free(a, omp_get_default_allocator());
    //    omp_free(b, omp_get_default_allocator());
    //    omp_free(c, omp_get_default_allocator());
    //    omp_free(d, omp_get_default_allocator());
    //
    stream_free(a);
    stream_free(b);
    stream_free(c);
    stream_free(d);
  }

  double avg_clock_axpy = average(clock_axpy, benchmark_repetitions);
  double avg_clock_fma = average(clock_fma, benchmark_repetitions);
  double avg_clock_copy = average(clock_copy, benchmark_repetitions);
  double avg_clock_addmul = average(clock_addmul, benchmark_repetitions);

  double bandwidth_axpy = compute_bandwidth(1, 3, vec_size, //
                                            avg_clock_axpy, sizeof(float_type));
  double bandwidth_fma = compute_bandwidth(1, 3, vec_size, //
                                           avg_clock_fma, sizeof(float_type));
  double bandwidth_copy = compute_bandwidth(1, 2, vec_size, //
                                            avg_clock_copy, sizeof(float_type));
  double bandwidth_addmul =
      compute_bandwidth(1, 4,                                  //
                        vec_size,                              //
                        avg_clock_addmul, sizeof(float_type)); //

  printf("\n-----------------------------------------------------------\n");
  printf("RESULTS OpenMP\n");
  print_performance_metrics(bandwidth_axpy, avg_clock_axpy, bandwidth_fma,
                            avg_clock_fma, bandwidth_copy, avg_clock_copy,
                            bandwidth_addmul, avg_clock_addmul, to_GB);
  free(clock_axpy);
  free(clock_fma);
  free(clock_copy);
  free(clock_addmul);

  return 0;
}
