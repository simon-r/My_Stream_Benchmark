/**
my_stream
Copyright (C) 2023 Simone Riva

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

#include <mpi.h>
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

struct stream_results {
  double clock;
  double bandwidth;
  double consume_out;
  double total_streamed_memory;
};

struct stream_results make_stream_results() {
  struct stream_results results;

  results.clock = 0.0;
  results.bandwidth = 0.0;
  results.consume_out = 0.0;
  results.total_streamed_memory = 0.0;

  return results;
}

struct streams_args {
  size_t size;
  size_t vec_size_proc;
  size_t benchmark_repetitions;

  struct stream_results FMA;
  struct stream_results copy;
  struct stream_results axpy;
  struct stream_results add_mul;
};

struct streams_args make_stream_args(const size_t size,                    //
                                     const size_t vec_size_proc,           //
                                     const size_t benchmark_repetitions) { //

  struct streams_args args;
  args.size = size;
  args.vec_size_proc = vec_size_proc;
  args.benchmark_repetitions = benchmark_repetitions;

  args.FMA = make_stream_results();
  args.copy = make_stream_results();
  args.axpy = make_stream_results();
  args.add_mul = make_stream_results();

  return args;
}

void *stream_calloc(size_t __alignment, size_t vector_len, size_t type_size) {
  return (void *)aligned_alloc(__alignment, vector_len * type_size);
}

typedef float_type vector_type
    __attribute__((vector_size(VECTOR_LEN * sizeof(float_type)), //
                   aligned(sizeof(float_type))));                //

/**
 * @brief
 *
 * @param a
 * @param b
 * @param c
 * @param d
 * @param args
 */
void FMA_test(float_type *a, float_type *b, float_type *c, float_type *d,
              struct streams_args *args) {

  vector_type *a_vec = (vector_type *)(a);
  vector_type *b_vec = (vector_type *)(b);
  vector_type *c_vec = (vector_type *)(c);
  vector_type *d_vec = (vector_type *)(d);

  size_t size_vec = args->vec_size_proc / VECTOR_LEN;

  float_type consume = 0;

  struct timespec start, end;

  for (int r = 0; r < args->benchmark_repetitions; r++) {
    MPI_Barrier(MPI_COMM_WORLD);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < size_vec; i++) {
      d_vec[i] = a_vec[i] * b_vec[i] + c_vec[i];
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    MPI_Barrier(MPI_COMM_WORLD);

    args->FMA.clock += get_time(start, end);
    consume +=
        d[rand() % args->vec_size_proc] + a[rand() % args->vec_size_proc] +
        b[rand() % args->vec_size_proc] + c[rand() % args->vec_size_proc];
  }

  args->FMA.clock /= args->benchmark_repetitions;
  args->FMA.bandwidth = compute_bandwidth(1, 4, args->vec_size_proc,
                                          args->FMA.clock, sizeof(float_type));
  args->FMA.consume_out = consume;
  args->FMA.total_streamed_memory =
      args->vec_size_proc * 4 * sizeof(float_type);
}

/**
 * @brief
 *
 * @param a
 * @param b
 * @param c
 * @param d
 * @param args
 */
void copy_test(float_type *a, float_type *b, float_type *c, float_type *d,
               struct streams_args *args) {

  vector_type *a_vec = (vector_type *)(a);
  vector_type *d_vec = (vector_type *)(d);

  size_t size_vec = args->vec_size_proc / VECTOR_LEN;

  float_type consume = 0;
  struct timespec start, end;

  for (int r = 0; r < args->benchmark_repetitions; r++) {
    MPI_Barrier(MPI_COMM_WORLD);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < size_vec; i++) {
      d_vec[i] = a_vec[i];
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    MPI_Barrier(MPI_COMM_WORLD);

    args->copy.clock += get_time(start, end);
    consume +=
        d[rand() % args->vec_size_proc] + a[rand() % args->vec_size_proc];
  }

  // args->clock_copy /= args->benchmark_repetitions;
  args->copy.clock /= args->benchmark_repetitions;
  args->copy.bandwidth = compute_bandwidth(
      1, 2, args->vec_size_proc, args->copy.clock, sizeof(float_type));

  args->copy.consume_out = consume;
  args->copy.total_streamed_memory =
      args->vec_size_proc * 2 * sizeof(float_type);
}

/**
 * @brief
 *
 * @param a
 * @param b
 * @param c
 * @param d
 * @param args
 */
void axpy_test(float_type *a, float_type *b, float_type *c, float_type *d,
               struct streams_args *args) {

  vector_type *a_vec = (vector_type *)(a);
  vector_type *b_vec = (vector_type *)(b);

  vector_type *d_vec = (vector_type *)(d);

  size_t size_vec = args->vec_size_proc / VECTOR_LEN;

  float_type consume = 0;
  float_type alpha = 2.56;

  struct timespec start, end;

  for (int r = 0; r < args->benchmark_repetitions; r++) {
    MPI_Barrier(MPI_COMM_WORLD);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < size_vec; i++) {
      d_vec[i] = alpha * a_vec[i] + b_vec[i];
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    MPI_Barrier(MPI_COMM_WORLD);

    args->axpy.clock += get_time(start, end);
    consume += d[rand() % args->vec_size_proc] +
               a[rand() % args->vec_size_proc] +
               c[rand() % args->vec_size_proc];
  }

  args->axpy.clock /= args->benchmark_repetitions;
  args->axpy.bandwidth = compute_bandwidth(
      1, 3, args->vec_size_proc, args->axpy.clock, sizeof(float_type));
  args->axpy.consume_out = consume;
  args->axpy.total_streamed_memory =
      args->vec_size_proc * 4 * sizeof(float_type);
}

/**
 * @brief
 *
 * @param a
 * @param b
 * @param c
 * @param d
 * @param args
 */
void add_mul_test(float_type *a, float_type *b, float_type *c, float_type *d,
                  struct streams_args *args) {

  vector_type *a_vec = (vector_type *)(a);
  vector_type *b_vec = (vector_type *)(b);
  vector_type *c_vec = (vector_type *)(c);
  vector_type *d_vec = (vector_type *)(d);

  size_t size_vec = args->vec_size_proc / VECTOR_LEN;

  float_type consume = 0;

  struct timespec start, end;

  for (int r = 0; r < args->benchmark_repetitions; r++) {
    MPI_Barrier(MPI_COMM_WORLD);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < size_vec; i++) {
      d_vec[i] = a_vec[i] + b_vec[i];
      c_vec[i] = a_vec[i] * b_vec[i];
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    MPI_Barrier(MPI_COMM_WORLD);

    args->add_mul.clock += get_time(start, end);
    consume +=
        d[rand() % args->vec_size_proc] + a[rand() % args->vec_size_proc] +
        b[rand() % args->vec_size_proc] + c[rand() % args->vec_size_proc];
  }

  args->add_mul.clock /= args->benchmark_repetitions;
  args->add_mul.bandwidth = compute_bandwidth(
      1, 4, args->vec_size_proc, args->add_mul.clock, sizeof(float_type));
  args->add_mul.consume_out = consume;
  args->add_mul.total_streamed_memory =
      args->vec_size_proc * 4 * sizeof(float_type);
}

#define HLINE                                                                  \
  "------------------------------------------------------------------\n"

int main(int argc, char **argv) {

  // Init mpi
  MPI_Init(&argc, &argv);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // get process number
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    printf("\n");
    printf(HLINE);
    printf("Start My Stream [MPI]\n");
    printf(HLINE);
    printf("\n");

#ifdef COMPILER
    printf("Compiler: %s\n\n", COMPILER);
#endif

#ifdef ARCHITECTURE
    printf("Architecture: %s\n\n", ARCHITECTURE);
#endif
  }

  size_t vec_size = DEFAULT_TEST_SIZE;
  int benchmark_repetitions = BENCHMARK_REPETITIONS;

  if ((flag_exists(argc, (const char **)argv, "-h") |
       flag_exists(argc, (const char **)argv, "--help"))) {
    if (rank == 0) {
      printf("Usage: %s [options]\n", argv[0]);
      printf("Options:\n");
      printf(
          "  -h, --help                  Show this help message and exit.\n");
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
    }

    MPI_Finalize();
    return 0;
  }

  const int ai =
      find_command_line_arg_value_v2(argc, (const char **)argv, "-s");

  if (ai > 0) {

    if (is_number(argv[ai])) {
      vec_size = strtoul(argv[ai], NULL, 10);
      if (rank == 0)
        printf("User defined vector size: %lu\n", vec_size);

    } else {

      if (rank == 0)
        printf("!!!!!!!! Error: argument of -s is not numeric\n");

      MPI_Finalize();
      return 1;
    }
  }

  const int ri =
      find_command_line_arg_value_v2(argc, (const char **)argv, "-r");

  if (ri > 0) {
    if (is_number(argv[ri])) {
      benchmark_repetitions = atoi(argv[ri]);
      if (rank == 0)
        printf("User defined benchmark repetitions: %d\n",
               benchmark_repetitions);
    } else {
      if (rank == 0)
        printf("Error: argv -r is not numeric\n");

      MPI_Finalize();
      return 1;
    }
  }

  // get the number of cpu from open mp
  // const int nr_cpu = omp_get_num_procs();
  vec_size = vec_size / world_size;
  vec_size = ((vec_size - vec_size % VECTOR_LEN) + VECTOR_LEN) * world_size;
  size_t vec_size_proc = vec_size / world_size;

  // double to_MB = (1024.0 * 1024.0);
  // double to_GB = (1024.0 * 1024.0 * 1024.0);

  double bytes_vec_size = (double)(vec_size_proc * sizeof(float_type));
  double MB_vec_size = bytes_vec_size / to_MB;
  double GB_vec_size = bytes_vec_size / to_GB;

  if (rank == 0) {
    printf("\n");
    printf(HLINE);
    printf("Number of MPI processes:               %d\n", world_size);
    printf("Adjusted vector size:                  %lu elements\n", vec_size);
    printf("MB Vector size per process:            %f MB\n", MB_vec_size);
    printf("GB Vector size per process:            %f GB\n", GB_vec_size);
    printf("GB Total allocated memory:             %f GB\n",
           (GB_vec_size * 4 * world_size));
    printf("Repetitions:                           %d\n",
           benchmark_repetitions);
    printf(HLINE);
    printf("\n");
  }

  struct streams_args *args =
      (struct streams_args *)malloc(world_size * sizeof(struct streams_args));

  args[rank] = make_stream_args(vec_size, vec_size_proc, benchmark_repetitions);

  float_type *a = (float_type *)stream_calloc(
      VECTOR_LEN * sizeof(float_type), vec_size_proc, sizeof(float_type));

  float_type *b = (float_type *)stream_calloc(
      VECTOR_LEN * sizeof(float_type), vec_size_proc, sizeof(float_type));

  float_type *c = (float_type *)stream_calloc(
      VECTOR_LEN * sizeof(float_type), vec_size_proc, sizeof(float_type));

  float_type *d = (float_type *)stream_calloc(
      VECTOR_LEN * sizeof(float_type), vec_size_proc, sizeof(float_type));

  unsigned int r = 1;
  for (int i = 0; i < vec_size_proc; i++) {
    r = generate_random_number(r);
    a[i] = 1.0 + (float_type)(r % 300) / 200.0;
    b[i] = 1.0 + (float_type)(r % 400) / 300.0;
    c[i] = 1.0 + (float_type)(r % 500) / 300.0;
    d[i] = 0.0;
  }

  MPI_Barrier(MPI_COMM_WORLD);
  FMA_test(a, b, c, d, &args[rank]);

  MPI_Barrier(MPI_COMM_WORLD);
  copy_test(a, b, c, d, &args[rank]);

  MPI_Barrier(MPI_COMM_WORLD);
  axpy_test(a, b, c, d, &args[rank]);

  MPI_Barrier(MPI_COMM_WORLD);
  add_mul_test(a, b, c, d, &args[rank]);

  // if (rank == 0) {
  //   for (int i = 1; i < world_size; i++) {
  //     MPI_Recv(&args[i], sizeof(struct streams_args), MPI_CHAR, i, 0,
  //              MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  //   }
  // } else {
  //   MPI_Send(&args[rank], sizeof(struct streams_args), MPI_CHAR, 0, 0,
  //            MPI_COMM_WORLD);
  // }

  // Gather all args from all processes to rank 0
  MPI_Gather(&args[rank],                 // Send buffer (local args)
             sizeof(struct streams_args), // Send count (size of one struct)
             MPI_BYTE,                    // Send type (use BYTE for structs)
             args, // Receive buffer (full array on rank 0)
             sizeof(struct streams_args), // Receive count per process
             MPI_BYTE,                    // Receive type
             0,                           // Root process
             MPI_COMM_WORLD);             // Communicator

  double FMA_total_bandwidth = 0.0;
  double copy_total_bandwidth = 0.0;
  double axpy_total_bandwidth = 0.0;
  double add_mul_total_bandwidth = 0.0;

  double clock_FMA = 0.0;
  double clock_copy = 0.0;
  double clock_axpy = 0.0;
  double clock_add_mul = 0.0;

  if (rank == 0) {

    for (int i = 0; i < world_size; i++) {
      FMA_total_bandwidth += (args[i].FMA.bandwidth / to_GB);
      clock_FMA += args[i].FMA.clock;

      copy_total_bandwidth += (args[i].copy.bandwidth / to_GB);
      clock_copy += args[i].copy.clock;

      axpy_total_bandwidth += (args[i].axpy.bandwidth / to_GB);
      clock_axpy += args[i].axpy.clock;

      add_mul_total_bandwidth += (args[i].add_mul.bandwidth / to_GB);
      clock_add_mul += args[i].add_mul.clock;
    }

    clock_FMA /= world_size;
    clock_copy /= world_size;
    clock_axpy /= world_size;
    clock_add_mul /= world_size;

    printf("Results:\n");
    printf(HLINE);
    printf("Test            Total bandwidth        clock  \n");
    printf(HLINE);
    printf("FMA:            %8.3f GB/s,          %5.3f ms\n",
           FMA_total_bandwidth, clock_FMA);
    printf("copy:           %8.3f GB/s,          %5.3f ms\n",
           copy_total_bandwidth, clock_copy);
    printf("axpy (TRIAD):   %8.3f GB/s,          %5.3f ms\n",
           axpy_total_bandwidth, clock_axpy);
    printf("add mul:        %8.3f GB/s,          %5.3f ms\n",
           add_mul_total_bandwidth, clock_add_mul);

    printf("\n");
    printf(HLINE);
  }

  free(a);
  free(b);
  free(c);
  free(d);

  MPI_Finalize();
  return 0;
}
