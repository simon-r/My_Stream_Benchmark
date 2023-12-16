#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define DFAULT_TEST_SIZE 1000000

unsigned int generate_random_number(unsigned int seed) {
  unsigned int magic = 214013;
  unsigned int m = 1 << 31;
  unsigned int a = 16807;

  return (seed * a + magic) % m;
}

double copy_benchmark(const size_t vec_size, const int nr_cpu) {}

double saxpy_benchmark(const size_t vec_size, const int nr_cpu) {}

int main(int argc, char **argv) {
  printf("Start My Stream\n------------------------\n\n");

  size_t vec_size = DFAULT_TEST_SIZE;

  if (argc >= 2) {
    vec_size = atoi(argv[1]);
  }

  printf("Vector size: %lu\n", vec_size);

  // get the number of cpu from open mp
  const int n_cpu = omp_get_num_procs();

  printf("Number of CPU: %d\n", n_cpu);

  return 0;
}