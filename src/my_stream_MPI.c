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

struct streams_args {
  size_t size;

  double clock_FMA;
  double bandwidth_FMA;
  double consume_out_FMA;

  size_t benchmark_repetitions;
};

void *stream_calloc(size_t __alignment, size_t vector_len, size_t type_size) {
  return (void *)aligned_alloc(__alignment, vector_len * type_size);
}

typedef float_type vector_type
    __attribute__((vector_size(VECTOR_LEN * sizeof(float_type)), //
                   aligned(sizeof(float_type))));                //

void FMA_test(float_type *a, float_type *b, float_type *c, float_type *d,
              struct streams_args *args, int nr_proc) {

  vector_type *a_vec = (vector_type *)(a);
  vector_type *b_vec = (vector_type *)(b);
  vector_type *c_vec = (vector_type *)(c);
  vector_type *d_vec = (vector_type *)(d);

  size_t size_vec = args->size / VECTOR_LEN;

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

    args->clock_FMA += get_time(start, end);
    consume += d[rand() % args->size] + a[rand() % args->size] +
               b[rand() % args->size] + c[rand() % args->size];
  }

  args->clock_FMA /= args->benchmark_repetitions;
  args->bandwidth_FMA =
      compute_bandwidth(1, 4, args->size, args->clock_FMA, sizeof(float_type));
  args->consume_out_FMA = consume;
}

int main(int argc, char **argv) {

  // Init mpi
  MPI_Init(&argc, &argv);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // get process number
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    printf("Start My Stream [MPI]\n------------------------\n\n");
  }

  size_t vec_size = DEFAULT_TEST_SIZE;
  int benchmark_repetitions = BENCHMARK_REPETITIONS;

  if ((flag_exists(argc, argv, "-h") | flag_exists(argc, argv, "--help"))) {
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
             ", is designed to benchmark the memory bandwidth (in Mb/s and "
             "Gb/s). \n"
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

  double to_Mb = (1024.0 * 1024.0);
  double to_Gb = (1024.0 * 1024.0 * 1024.0);

  double bytes_vec_size = (double)(vec_size * sizeof(float_type));
  double Mb_vec_size = bytes_vec_size / to_Mb;
  double Gb_vec_size = bytes_vec_size / to_Gb;

  if (rank == 0) {
    printf("\n-----------------------------------------------------------\n");
    printf("Number of CPU:             %d\n", world_size);
    printf("Adjusted vector size:      %lu\n", vec_size);
    printf("Mb Vector size:            %f\n", Mb_vec_size);
    printf("Gb Vector size:            %f\n", Gb_vec_size);
    printf("Gb Total allocated memory: %f\n", Gb_vec_size * 4);
    printf("Repetitions:               %d\n", benchmark_repetitions);
    printf("-----------------------------------------------------------\n\n");
  }

  struct streams_args *args =
      (struct streams_args *)malloc(world_size * sizeof(struct streams_args));

  args[rank].size = vec_size;
  args[rank].benchmark_repetitions = benchmark_repetitions;
  args[rank].clock_FMA = 0;
  args[rank].bandwidth_FMA = 0;
  args[rank].consume_out_FMA = 0;

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

  MPI_Barrier(MPI_COMM_WORLD);
  FMA_test(a, b, c, d, &args[rank], world_size);

  if (rank == 0) {
    for (int i = 1; i < world_size; i++) {
      MPI_Recv(&args[i], sizeof(struct streams_args), MPI_CHAR, i, 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  } else {
    MPI_Send(&args[rank], sizeof(struct streams_args), MPI_CHAR, 0, 0,
             MPI_COMM_WORLD);
  }

  double FMA_total_bandwidth = 0;
  if (rank == 0) {

    for (int i = 0; i < world_size; i++) {
      printf("FMA - Rank:            %d\n", i);
      printf("FMA - Size:            %lu\n", args[i].size);
      printf("FMA - Clock:           %f\n", args[i].clock_FMA);
      printf("FMA - Bandwidth:       %f\n", args[i].bandwidth_FMA / to_Gb);
      printf("FMA - Consume out:     %f\n", args[i].consume_out_FMA);
      printf("FMA - Repetitions:     %ld\n", args[i].benchmark_repetitions);
      printf("\n");
      FMA_total_bandwidth += (args[i].bandwidth_FMA / to_Gb);
      printf("-----------------------------------------------------------\n");
      printf("\n");
    }
  }

  if (rank == 0)
    printf("FMA - Total bandwidth: %f GB/s\n", FMA_total_bandwidth);

  free(a);
  free(b);
  free(c);
  free(d);

  MPI_Finalize();
  return 0;
}
