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

#define DEFAULT_TEST_SIZE 50000000

#define VECTOR_LEN 8

#define BENCHMARK_REPETITIONS 50

#define VERBOSE
#undef VERBOSE

typedef double float_type;

struct streams_args {
  size_t size;

  double clock;
  double bandwidth;

  double consume_out;
  size_t benchmark_repetitions;
};

int main(int argc, char **argv) {

  // Init mpi
  MPI_Init(&argc, &argv);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // get process number
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    printf("Start My Stream [Multi Threads - Global "
           "Memory]\n------------------------\n\n");
  }

  size_t vec_size = DEFAULT_TEST_SIZE;
  int benchmark_repetitions = BENCHMARK_REPETITIONS;

  if ((flag_exists(argc, argv, "-h") | flag_exists(argc, argv, "--help")) &
      rank == 0) {

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
      if (rank == 0)
        printf("User defined vector size: %lu\n", vec_size);
    } else {
      if (rank == 0)
        printf("Error: argument of -s is not numeric\n");
      return 1;
    }
  }

  char *benchmark_repetitions_arg =
      find_command_line_arg_value(argc, argv, "-r");

  if (benchmark_repetitions_arg != NULL) {
    if (is_number(benchmark_repetitions_arg)) {
      benchmark_repetitions = atoi(benchmark_repetitions_arg);
      if (rank == 0)
        printf("User defined benchmark repetitions: %d\n",
               benchmark_repetitions);
    } else {
      if (rank == 0)
        printf("Error: argv -r is not numeric\n");
      return 1;
    }
  }

  // get the number of cpu from open mp
  const int nr_cpu = omp_get_num_procs();
  vec_size = vec_size / nr_cpu;
  vec_size = ((vec_size - vec_size % VECTOR_LEN) + VECTOR_LEN) * nr_cpu;

  double to_Mb = (1024.0 * 1024.0);
  double to_Gb = (1024.0 * 1024.0 * 1024.0);

  double bytes_vec_size = (double)(vec_size * sizeof(float_type));
  double Mb_vec_size = bytes_vec_size / to_Mb;
  double Gb_vec_size = bytes_vec_size / to_Gb;

  if (rank == 0) {
    printf("\n-----------------------------------------------------------\n");
    printf("Number of CPU:             %d\n", nr_cpu);
    printf("Adjusted vector size:      %lu\n", vec_size);
    printf("Mb Vector size:            %f\n", Mb_vec_size);
    printf("Gb Vector size:            %f\n", Gb_vec_size);
    printf("Gb Total allocated memory: %f\n", Gb_vec_size * 4);
    printf("Repetitions:               %d\n", benchmark_repetitions);
    printf("-----------------------------------------------------------\n\n");
  }

  struct streams_args *args =
      (struct streams_args *)malloc(world_size * sizeof(struct streams_args));

  // malloc a aligned to 4 * sizeof(float_type)
  float_type *a = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size, sizeof(float_type));
  float_type *b = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size, sizeof(float_type));
  float_type *c = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size, sizeof(float_type));
  float_type *d = (float_type *)stream_calloc(VECTOR_LEN * sizeof(float_type),
                                              vec_size, sizeof(float_type));

  free(a);
  free(b);
  free(c);
  free(d);

  MPI_Finalize();
  return 0;
}
