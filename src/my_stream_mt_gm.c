/**
my_stream
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
 * @version 0.5
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

sem_t semaphore;

typedef float_type vector_type
    __attribute__((vector_size(VECTOR_LEN * sizeof(float_type)), //
                   aligned(sizeof(float_type))));                //

struct streams_args {
  float_type *a;
  float_type *b;
  float_type *c;
  float_type *d;

  size_t start_index;
  size_t end_index;

  double clock;
};

#define MAKE_BENCHMARK_FUNC(FUNC_NAME, BENCHMARK_FUN)                          \
  double FUNC_NAME(const size_t vec_size, const int nr_cpu,                    \
                   struct streams_args *threads_args) {                        \
                                                                               \
    /** make a vector of pthreads*/                                            \
    pthread_t *threads = malloc(nr_cpu * sizeof(pthread_t));                   \
                                                                               \
    /* Initialize semaphore*/                                                  \
    sem_init(&semaphore, 0, nr_cpu);                                           \
                                                                               \
    for (int i = 0; i < nr_cpu; i++) {                                         \
      pthread_create(&threads[i], NULL, BENCHMARK_FUN,                         \
                     (void *)(&threads_args[i]));                              \
    }                                                                          \
                                                                               \
    for (int i = 0; i < nr_cpu; i++) {                                         \
      pthread_join(threads[i], NULL);                                          \
    }                                                                          \
                                                                               \
    double average_time = 0;                                                   \
                                                                               \
    for (int i = 0; i < nr_cpu; i++) {                                         \
      average_time += threads_args[i].clock;                                   \
    }                                                                          \
                                                                               \
    average_time /= nr_cpu;                                                    \
                                                                               \
    free(threads);                                                             \
    return average_time;                                                       \
  }

/**
 * @brief
 *
 * @param arg_void
 * @return void*
 */
void *fma_thread(void *arg_void) {

  struct streams_args *threads_args = (struct streams_args *)arg_void;

  vector_type *a_vec =
      (vector_type *)(threads_args->a + threads_args->start_index);
  vector_type *b_vec =
      (vector_type *)(threads_args->b + threads_args->start_index);
  vector_type *c_vec =
      (vector_type *)(threads_args->c + threads_args->start_index);

  vector_type *d_vec =
      (vector_type *)(threads_args->d + threads_args->start_index);

  size_t size_vec =
      (threads_args->end_index - threads_args->start_index) / VECTOR_LEN;

  // start hi definition clock
  struct timespec start, end;

  // printf("size_vec %d\n", size_vec);

  sem_wait(&semaphore);

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < size_vec; i++) {
    d_vec[i] = a_vec[i] * b_vec[i] + c_vec[i];
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  // Calculate elapsed time in milliseconds

  double elapsed = (double)(end.tv_sec - start.tv_sec) *
                   (double)1000LL; // Convert seconds to milliseconds
  elapsed += (double)(end.tv_nsec - start.tv_nsec) /
             (double)1000000LL; // Convert nanoseconds to milliseconds

  // printf("Elapsed time: %lf milliseconds\n", elapsed);
  threads_args->clock = elapsed;

  return NULL;
}

/**
 * @brief
 *
 * @param arg_void
 * @return void*
 */
void *copy_thread(void *arg_void) {

  struct streams_args *threads_args = (struct streams_args *)arg_void;

  vector_type *a_vec =
      (vector_type *)(threads_args->a + threads_args->start_index);

  vector_type *d_vec =
      (vector_type *)(threads_args->d + threads_args->start_index);

  size_t size_vec =
      (threads_args->end_index - threads_args->start_index) / VECTOR_LEN;

  // start hi definition clock
  struct timespec start, end;

  // printf("size_vec %d\n", size_vec);

  sem_wait(&semaphore);

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < size_vec; i++) {
    d_vec[i] = a_vec[i];
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  // Calculate elapsed time in milliseconds

  double elapsed = (double)(end.tv_sec - start.tv_sec) *
                   (double)1000LL; // Convert seconds to milliseconds

  elapsed += (double)(end.tv_nsec - start.tv_nsec) /
             (double)1000000LL; // Convert nanoseconds to milliseconds

  // printf("Elapsed time: %lf milliseconds\n", elapsed);
  threads_args->clock = elapsed;

  return NULL;
}

/**
 * @brief
 *
 * @param arg_void
 * @return void*
 */
void *axpy_thread(void *arg_void) {

  struct streams_args *threads_args = (struct streams_args *)arg_void;

  float_type alpha = 2.55;

  vector_type *a_vec =
      (vector_type *)(threads_args->a + threads_args->start_index);
  vector_type *b_vec =
      (vector_type *)(threads_args->b + threads_args->start_index);

  vector_type *d_vec =
      (vector_type *)(threads_args->d + threads_args->start_index);

  size_t size_vec =
      (threads_args->end_index - threads_args->start_index) / VECTOR_LEN;

  // start hi definition clock
  struct timespec start, end;

  // printf("size_vec %d\n", size_vec);

  sem_wait(&semaphore);

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < size_vec; i++) {
    d_vec[i] = alpha * a_vec[i] + b_vec[i];
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  // Calculate elapsed time in milliseconds

  double elapsed = (double)(end.tv_sec - start.tv_sec) *
                   (double)1000LL; // Convert seconds to milliseconds
  elapsed += (double)(end.tv_nsec - start.tv_nsec) /
             (double)1000000LL; // Convert nanoseconds to milliseconds

  // printf("Elapsed time: %lf milliseconds\n", elapsed);
  threads_args->clock = elapsed;

  return NULL;
}

void *add_mult_thread(void *arg_void) {

  struct streams_args *threads_args = (struct streams_args *)arg_void;

  vector_type *a_vec =
      (vector_type *)(threads_args->a + threads_args->start_index);
  vector_type *b_vec =
      (vector_type *)(threads_args->b + threads_args->start_index);

  vector_type *c_vec =
      (vector_type *)(threads_args->c + threads_args->start_index);
  vector_type *d_vec =
      (vector_type *)(threads_args->d + threads_args->start_index);

  size_t size_vec =
      (threads_args->end_index - threads_args->start_index) / VECTOR_LEN;

  // start hi definition clock
  struct timespec start, end;

  // printf("size_vec %d\n", size_vec);

  sem_wait(&semaphore);

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < size_vec; i++) {
    d_vec[i] = a_vec[i] + b_vec[i];
    c_vec[i] = a_vec[i] * b_vec[i];
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  // Calculate elapsed time in milliseconds

  double elapsed = (double)(end.tv_sec - start.tv_sec) *
                   (double)1000LL; // Convert seconds to milliseconds
  elapsed += (double)(end.tv_nsec - start.tv_nsec) /
             (double)1000000LL; // Convert nanoseconds to milliseconds

  // printf("Elapsed time: %lf milliseconds\n", elapsed);
  threads_args->clock = elapsed;

  return NULL;
}

MAKE_BENCHMARK_FUNC(axpy_benchmark, axpy_thread)

MAKE_BENCHMARK_FUNC(copy_benchmark, copy_thread)

MAKE_BENCHMARK_FUNC(fma_benchmark, fma_thread)

MAKE_BENCHMARK_FUNC(add_mult_benchmark, add_mult_thread)

void *stream_calloc(size_t __alignment, size_t vector_len, size_t type_size) {
  return (void *)aligned_alloc(__alignment, vector_len * type_size);
}

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(const int argc, const char **argv) {

  printf("Start My Stream [Multi Threads - Global "
         "Memory]\n------------------------\n\n");

#ifdef COMPILER
  printf("Compiler: %s\n\n", COMPILER);
#endif

#ifdef ARCHITECTURE
  printf("Architecture: %s\n\n", ARCHITECTURE);
#endif

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
           ", is designed to benchmark the memory bandwidth (in MB/s and "
           "GB/s). \n"
           "  In order to measure the bandwidth, it executes four "
           "\"memory bound\" vector operations: Axpy, Copy, FMA (fused "
           "multiply-add), and Add Mult..\n"
           "  Visit: https://github.com/simon-r/My_Stream_Benchmark \n");

    printf("\n");
    return 0;
  }

  const char *vec_size_arg = find_command_line_arg_value(argc, argv, "-s");

  if (vec_size_arg != NULL) {

    if (is_number(vec_size_arg)) {
      vec_size = strtoul(vec_size_arg, NULL, 10);
      printf("User defined vector size: %lu\n", vec_size);
    } else {
      printf("Error: argument of -s is not numeric\n");
      return 1;
    }
  }

  const char *benchmark_repetitions_arg =
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

  // double to_MB = (1024.0 * 1024.0);
  // double to_GB = (1024.0 * 1024.0 * 1024.0);

  double bytes_vec_size = (double)(vec_size * sizeof(float_type));
  double MB_vec_size = bytes_vec_size / to_MB;
  double GB_vec_size = bytes_vec_size / to_GB;

  printf("\n-----------------------------------------------------------\n");
  printf("Number of CPU:             %d\n", nr_cpu);
  printf("Adjusted vector size:      %lu\n", vec_size);
  printf("MB Vector size:            %f\n", MB_vec_size);
  printf("GB Vector size:            %f\n", GB_vec_size);
  printf("GB Total allocated memory: %f\n", GB_vec_size * 4);
  printf("Repetitions:               %d\n", benchmark_repetitions);
  printf("-----------------------------------------------------------\n\n");

  // malloc a aligned to 4 * sizeof(float_type)
  float_type *a = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size, sizeof(float_type));
  float_type *b = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size, sizeof(float_type));
  float_type *c = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size, sizeof(float_type));
  float_type *d = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size, sizeof(float_type));

  unsigned int r = 1;
  for (int i = 0; i < vec_size; i++) {
    r = generate_random_number(r);
    a[i] = 1.0 + (float_type)(r % 300) / 200.0;
    b[i] = 1.0 + (float_type)(r % 400) / 300.0;
    c[i] = 1.0 + (float_type)(r % 500) / 300.0;
    d[i] = 0.0;
  }

  struct streams_args *th_args = malloc(nr_cpu * sizeof(struct streams_args));

  size_t batch_vec_size = vec_size / nr_cpu;

  for (int i = 0; i < nr_cpu; i++) {
    th_args[i].a = a;
    th_args[i].b = b;
    th_args[i].c = c;
    th_args[i].d = d;
    th_args[i].start_index = i * batch_vec_size;
    th_args[i].end_index = (i + 1) * batch_vec_size;
  }

  double consume = 0.0;
  double average_axpy_time = 0.0;

  for (int i = 0; i < benchmark_repetitions; i++) {
    average_axpy_time += axpy_benchmark(vec_size, nr_cpu, th_args);
    consume += a[100] + b[1002] + c[1002] + d[1002];
  }

  average_axpy_time /= (double)(benchmark_repetitions);

  double memory_streamed_axpy_MB =
      (3.0 * batch_vec_size * nr_cpu * sizeof(float_type)) / to_MB *
      benchmark_repetitions;
  // double memory_streamed_axpy_GB =
  //     (3.0 * batch_vec_size * nr_cpu * sizeof(float_type)) / to_GB *
  //     benchmark_repetitions;

  double bandwidth_axpy = (3.0 * batch_vec_size * nr_cpu * sizeof(float_type)) /
                          (average_axpy_time / 1000.0);

  // double bandwidth_axpy_MBS = bandwidth_axpy / to_MB;
  double bandwidth_axpy_GBS = bandwidth_axpy / to_GB;

#ifdef VERBOSE
  printf("Average axpy Time: %lf\n", average_axpy_time);
  printf("Bandwidth axpy: %lf MB/s\n", bandwidth_axpy_MBS);
  printf("Bandwidth axpy: %lf GB/s\n", bandwidth_axpy_GBS);
#endif // VERBOSE

  // printf("Consume axpy: %lf\n", consume);
  // printf("-----------------------------------------------------------\n\n");

  double average_copy_time = 0.0;
  for (int i = 0; i < benchmark_repetitions; i++) {
    average_copy_time += copy_benchmark(vec_size, nr_cpu, th_args);
    consume += a[100] + b[1002] + c[1002] + d[1002];
  }
  average_copy_time /= (double)(benchmark_repetitions);

  double memory_streamed_copy_MB =
      (2.0 * batch_vec_size * nr_cpu * sizeof(float_type)) / to_MB *
      benchmark_repetitions;
  // double memory_streamed_copy_GB =
  //     (2.0 * batch_vec_size * nr_cpu * sizeof(float_type)) / to_GB *
  //     benchmark_repetitions;

  double bandwidth_copy = (2.0 * batch_vec_size * nr_cpu * sizeof(float_type)) /
                          (average_copy_time / 1000.0);

  // double bandwidth_copy_MBS = bandwidth_copy / to_MB;
  double bandwidth_copy_GBS = bandwidth_copy / to_GB;

#ifdef VERBOSE
  printf("Average Copy Time: %lf\n", average_copy_time);
  printf("Bandwidth copy: %lf MB/s\n", bandwidth_copy_MBS);
  printf("Bandwidth copy: %lf GB/s\n", bandwidth_copy_GBS);
#endif // VERBOSE

  double average_fma_time = 0.0;
  for (int i = 0; i < benchmark_repetitions; i++) {
    average_fma_time += fma_benchmark(vec_size, nr_cpu, th_args);
    consume += a[100] + b[1002] + c[1002] + d[1002];
  }
  average_fma_time /= (double)(benchmark_repetitions);

  double memory_streamed_fma_MB =
      (4.0 * batch_vec_size * nr_cpu * sizeof(float_type)) / to_MB *
      benchmark_repetitions;

  // double memory_streamed_fma_GB =
  //     (4.0 * batch_vec_size * nr_cpu * sizeof(float_type)) / to_GB *
  //     benchmark_repetitions;

  double bandwidth_fma = (4.0 * batch_vec_size * nr_cpu * sizeof(float_type)) /
                         (average_fma_time / 1000.0);

  // double bandwidth_fma_MBS = bandwidth_fma / to_MB;
  double bandwidth_fma_GBS = bandwidth_fma / to_GB;

#ifdef VERBOSE
  printf("Average FMA Time: %lf\n", average_fma_time);
  printf("Bandwidth fma: %lf MB/s\n", bandwidth_fma_MBS);
  printf("Bandwidth fma: %lf GB/s\n", bandwidth_fma_GBS);
#endif // VERBOSE

  double average_add_mult_time = 0.0;
  for (int i = 0; i < benchmark_repetitions; i++) {
    average_add_mult_time += add_mult_benchmark(vec_size, nr_cpu, th_args);
    consume += a[100] + b[1002] + c[1002] + d[1002];
  }
  average_add_mult_time /= (double)(benchmark_repetitions);

  double memory_streamed_add_mult_MB =
      (4.0 * batch_vec_size * nr_cpu * sizeof(float_type)) / to_MB *
      benchmark_repetitions;

  // double memory_streamed_add_mult_GB =
  //     (4.0 * batch_vec_size * nr_cpu * sizeof(float_type)) / to_GB *
  //     benchmark_repetitions;

  double bandwidth_add_mult =
      (4.0 * batch_vec_size * nr_cpu * sizeof(float_type)) /
      (average_add_mult_time / 1000.0);

  // double bandwidth_add_mult_MBS = bandwidth_add_mult / to_MB;
  double bandwidth_add_mult_GBS = bandwidth_add_mult / to_GB;

#ifdef VERBOSE
  printf("Average Add Mult Time: %lf\n", average_add_mult_time);
  printf("Bandwidth add mult: %lf MB/s\n", bandwidth_add_mult_MBS);
  printf("Bandwidth add mult: %lf GB/s\n", bandwidth_add_mult_GBS);
#endif // VERBOSE

#define SEP                                                                    \
  "--------------------------------------------------------------------------" \
  "---------------------"                                                      \
  "\n\n"

  printf(SEP);

  printf("consume %f (just an output)\n", consume);

  printf(SEP);

  // printf("Results:\n");
  // printf(SEP);
  // printf("Benchmark:            GB/s                 Memory "
  //        "Streamed [MB]\n");
  // printf(SEP);
  // printf("Axpy:       %15.2lf [GB/s]         %lf\n",   //
  //        bandwidth_axpy_GBS, memory_streamed_axpy_MB); //

  // printf("Copy:       %15.2lf [GB/s]         %lf\n",   //
  //        bandwidth_copy_GBS, memory_streamed_copy_MB); //

  // printf("FMA:        %15.2lf [GB/s]         %lf\n", //
  //        bandwidth_fma_GBS, memory_streamed_fma_MB); //

  // printf("Add Mult:   %15.2lf [GB/s]         %lf\n",           //
  //        bandwidth_add_mult_GBS, memory_streamed_add_mult_MB); //

  printf("Results:\n");
  printf(SEP);
  printf("Benchmark:            GB/s                 Memory Streamed [MB]      "
         "Avg. Clock [ms]\n");
  printf(SEP);
  printf("Axpy:       %15.2lf [GB/s]         %lf                 %lf\n",  //
         bandwidth_axpy_GBS, memory_streamed_axpy_MB, average_axpy_time); //

  printf("Copy:       %15.2lf [GB/s]         %lf                 %lf\n",  //
         bandwidth_copy_GBS, memory_streamed_copy_MB, average_copy_time); //

  printf("FMA:        %15.2lf [GB/s]         %lf                 %lf\n", //
         bandwidth_fma_GBS, memory_streamed_fma_MB, average_fma_time);   //

  printf("Add Mult:   %15.2lf [GB/s]         %lf                 %lf\n", //
         bandwidth_add_mult_GBS, memory_streamed_add_mult_MB,
         average_add_mult_time); //

  printf(SEP);

  free(a);
  free(b);
  free(c);
  free(d);
  free(th_args);

  return 0;
}
