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
#include <time.h>

#define DEFAULT_TEST_SIZE 50000000

#define VECTOR_LEN 4

#define BENCHMARK_REPETITIONS 50

#define VERBOSE
#undef VERBOSE

typedef double float_type;

sem_t semaphore;

typedef float_type vector_type
    __attribute__((vector_size(VECTOR_LEN * sizeof(float_type)), //
                   aligned(sizeof(float_type))));                //

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

struct streams_args {
  float_type *a;
  float_type *b;
  float_type *c;
  float_type *d;

  size_t start_index;
  size_t end_index;

  double clock;
};

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
 * @param vec_size
 * @param nr_cpu
 * @return double
 */
double fma_benchmark(const size_t vec_size, const int nr_cpu,
                     struct streams_args *threads_args) {

  // make a vector of pthreads
  pthread_t *threads = malloc(nr_cpu * sizeof(pthread_t));

  // Initialize semaphore
  sem_init(&semaphore, 0, nr_cpu);

  for (int i = 0; i < nr_cpu; i++) {
    pthread_create(&threads[i], NULL, fma_thread, (void *)(&threads_args[i]));
  }

  for (int i = 0; i < nr_cpu; i++) {
    pthread_join(threads[i], NULL);
  }

  double average_time = 0;

  for (int i = 0; i < nr_cpu; i++) {
    average_time += threads_args[i].clock;
  }

  average_time /= nr_cpu;

  free(threads);
  return average_time;
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
 * @param vec_size
 * @param nr_cpu
 * @return double
 */
double copy_benchmark(const size_t vec_size, const int nr_cpu,
                      struct streams_args *threads_args) {

  // make a vector of pthreads
  pthread_t *threads = malloc(nr_cpu * sizeof(pthread_t));

  // Initialize semaphore
  sem_init(&semaphore, 0, nr_cpu);

  for (int i = 0; i < nr_cpu; i++) {
    pthread_create(&threads[i], NULL, copy_thread, (void *)(&threads_args[i]));
  }

  for (int i = 0; i < nr_cpu; i++) {
    pthread_join(threads[i], NULL);
  }

  double average_time = 0;

  for (int i = 0; i < nr_cpu; i++) {
    average_time += threads_args[i].clock;
  }

  average_time /= nr_cpu;

  free(threads);
  return average_time;
}

/**
 * @brief
 *
 * @param arg_void
 * @return void*
 */
void *axpy_thread(void *arg_void) {

  struct streams_args *threads_args = (struct streams_args *)arg_void;

  float_type aa = 2.55;

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
    d_vec[i] = aa * a_vec[i] + b_vec[i];
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
 * @param vec_size
 * @param nr_cpu
 * @param threads_args
 * @return double
 */
double axpy_benchmark(const size_t vec_size, const int nr_cpu,
                      struct streams_args *threads_args) {

  // make a vector of pthreads
  pthread_t *threads = malloc(nr_cpu * sizeof(pthread_t));

  // Initialize semaphore
  sem_init(&semaphore, 0, nr_cpu);

  for (int i = 0; i < nr_cpu; i++) {
    pthread_create(&threads[i], NULL, axpy_thread, (void *)(&threads_args[i]));
  }

  for (int i = 0; i < nr_cpu; i++) {
    pthread_join(threads[i], NULL);
  }

  double average_time = 0;

  for (int i = 0; i < nr_cpu; i++) {
    average_time += threads_args[i].clock;
  }

  average_time /= nr_cpu;

  free(threads);
  return average_time;
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

  if (argc >= 2) {
    vec_size = atoi(argv[1]);
  }

  printf("Vector size: %lu\n", vec_size);

  // get the number of cpu from open mp
  const int n_cpu = omp_get_num_procs();
  vec_size = vec_size / n_cpu;
  vec_size = ((vec_size - vec_size % VECTOR_LEN) + VECTOR_LEN) * n_cpu;

  double bytes_vec_size = (double)(vec_size * sizeof(float_type));
  double Mb_vec_size = bytes_vec_size / (1024.0 * 1024.0);
  double Gb_vec_size = bytes_vec_size / (1024.0 * 1024.0 * 1024.0);

  printf("Number of CPU:             %d\n", n_cpu);
  printf("Corrected vector size:     %lu\n", vec_size);
  printf("Mb Vector size:            %f\n", Mb_vec_size);
  printf("Gb Vector size:            %f\n", Gb_vec_size);
  printf("Gb Total allocated memory: %f\n", Gb_vec_size * 4);
  printf("Repetitions:               %d\n", BENCHMARK_REPETITIONS);
  printf("-----------------------------------------------------------\n\n");

  // malloc a aligned to 4 * sizeof(float_type)
  float_type *a = (float_type *)aligned_alloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size * sizeof(float_type));
  float_type *b = (float_type *)aligned_alloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size * sizeof(float_type));
  float_type *c = (float_type *)aligned_alloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size * sizeof(float_type));
  float_type *d = (float_type *)aligned_alloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size * sizeof(float_type));

  unsigned int r = 1;
  for (int i = 0; i < vec_size; i++) {
    r = generate_random_number(r);
    a[i] = 1.0 + (float_type)(r % 300) / 200.0;
    b[i] = 1.0 + (float_type)(r % 400) / 300.0;
    c[i] = 1.0 + (float_type)(r % 500) / 300.0;
    d[i] = 0.0;
  }

  struct streams_args *th_args = malloc(n_cpu * sizeof(struct streams_args));

  size_t batch_vec_size = vec_size / n_cpu;

  for (int i = 0; i < n_cpu; i++) {
    th_args[i].a = a;
    th_args[i].b = b;
    th_args[i].c = c;
    th_args[i].d = d;
    th_args[i].start_index = i * batch_vec_size;
    th_args[i].end_index = (i + 1) * batch_vec_size;
  }

  double consume = 0.0;
  double average_axpy_time = 0.0;

  for (int i = 0; i < BENCHMARK_REPETITIONS; i++) {
    average_axpy_time += axpy_benchmark(vec_size, n_cpu, th_args);
    consume += a[100] + b[1002] + c[1002] + d[1002];
  }

  average_axpy_time /= (double)(BENCHMARK_REPETITIONS);

  double memory_streamed_axpy_Mb =
      (3.0 * batch_vec_size * n_cpu * sizeof(float_type)) / (1024.0 * 1024.0) *
      BENCHMARK_REPETITIONS;
  double memory_streamed_axpy_Gb =
      (3.0 * batch_vec_size * n_cpu * sizeof(float_type)) /
      (1024.0 * 1024.0 * 1024.0) * BENCHMARK_REPETITIONS;

  double bandwidth_axpy = (3.0 * batch_vec_size * n_cpu * sizeof(float_type)) /
                          (average_axpy_time / 1000.0);

  double bandwidth_axpy_MbS = bandwidth_axpy / (1024.0 * 1024.0);
  double bandwidth_axpy_GbS = bandwidth_axpy / (1024.0 * 1024.0 * 1024.0);

#ifdef VERBOSE
  printf("Average axpy Time: %lf\n", average_axpy_time);
  printf("Bandwidth axpy: %lf Mb/s\n", bandwidth_axpy_MbS);
  printf("Bandwidth axpy: %lf Gb/s\n", bandwidth_axpy_GbS);
#endif // VERBOSE

  // printf("Consume axpy: %lf\n", consume);
  // printf("-----------------------------------------------------------\n\n");

  double average_copy_time = 0.0;
  for (int i = 0; i < BENCHMARK_REPETITIONS; i++) {
    average_copy_time += copy_benchmark(vec_size, n_cpu, th_args);
    consume += a[100] + b[1002] + c[1002] + d[1002];
  }
  average_copy_time /= (double)(BENCHMARK_REPETITIONS);

  double memory_streamed_copy_Mb =
      (2.0 * batch_vec_size * n_cpu * sizeof(float_type)) / (1024.0 * 1024.0) *
      BENCHMARK_REPETITIONS;
  double memory_streamed_copy_Gb =
      (2.0 * batch_vec_size * n_cpu * sizeof(float_type)) /
      (1024.0 * 1024.0 * 1024.0) * BENCHMARK_REPETITIONS;

  double bandwidth_copy = (2.0 * batch_vec_size * n_cpu * sizeof(float_type)) /
                          (average_copy_time / 1000.0);

  double bandwidth_copy_MbS = bandwidth_copy / (1024.0 * 1024.0);
  double bandwidth_copy_GbS = bandwidth_copy / (1024.0 * 1024.0 * 1024.0);

#ifdef VERBOSE
  printf("Average Copy Time: %lf\n", average_copy_time);
  printf("Bandwidth copy: %lf Mb/s\n", bandwidth_copy_MbS);
  printf("Bandwidth copy: %lf Gb/s\n", bandwidth_copy_GbS);
#endif // VERBOSE

  double average_fma_time = 0.0;
  for (int i = 0; i < BENCHMARK_REPETITIONS; i++) {
    average_fma_time += fma_benchmark(vec_size, n_cpu, th_args);
    consume += a[100] + b[1002] + c[1002] + d[1002];
  }
  average_fma_time /= (double)(BENCHMARK_REPETITIONS);

  double memory_streamed_fma_Mb =
      (4.0 * batch_vec_size * n_cpu * sizeof(float_type)) / (1024.0 * 1024.0) *
      BENCHMARK_REPETITIONS;

  double memory_streamed_fma_Gb =
      (4.0 * batch_vec_size * n_cpu * sizeof(float_type)) /
      (1024.0 * 1024.0 * 1024.0) * BENCHMARK_REPETITIONS;

  double bandwidth_fma = (4.0 * batch_vec_size * n_cpu * sizeof(float_type)) /
                         (average_fma_time / 1000.0);

  double bandwidth_fma_MbS = bandwidth_fma / (1024.0 * 1024.0);
  double bandwidth_fma_GbS = bandwidth_fma / (1024.0 * 1024.0 * 1024.0);

#ifdef VERBOSE
  printf("Average FMA Time: %lf\n", average_fma_time);
  printf("Bandwidth fma: %lf Mb/s\n", bandwidth_fma_MbS);
  printf("Bandwidth fma: %lf Gb/s\n", bandwidth_fma_GbS);
#endif // VERBOSE

  printf("consume %f\n", consume);

  printf("---------------------------------------------------------------------"
         "\n\n");

  printf("Results:\n");
  printf("---------------------------------------------------------------------"
         "\n\n");
  printf("Benchmark:      Mb/s           Gb/s          Memory Streamed [Mb]\n");
  printf("---------------------------------------------------------------------"
         "\n");
  printf("Axpy:           %.2lf       %.2lf         %lf\n", //
         bandwidth_axpy_MbS,                                //
         bandwidth_axpy_GbS, memory_streamed_axpy_Mb);      //

  printf("Copy:           %.2lf       %.2lf         %lf\n", //
         bandwidth_copy_MbS,                                 //
         bandwidth_copy_GbS, memory_streamed_copy_Mb);       //

  printf("FMA:            %.2lf       %.2lf         %lf\n", //
         bandwidth_fma_MbS,                                  //
         bandwidth_fma_GbS, memory_streamed_fma_Mb);         //

  printf("---------------------------------------------------------------------"
         "\n\n");

  free(a);
  free(b);
  free(c);
  free(d);

  return 0;
}