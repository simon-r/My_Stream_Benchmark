# My Stream Benchmark

-------------------------

**My_Stream**, is a collection of benchmarks designed to test memory bandwidth (in Mb/s and Gb/s). To measure the bandwidth, it performs four "memory bound" vector operations: Axpy (aka TRIAD), Copy, FMA (fused multiply-add), and Add Mult.

***Version: 0.2.0***

### Key Components

* Libraries included: OpenMP, MPI, pthread, semaphore, stdio, stdlib, time.
* Data types: It uses a typedef float_type as double.
* Vector operations: Uses SIMD (Single Instruction, Multiple Data) capabilities to efficiently process vectors.
* MPI (OpenMPI or MPICH)

### Results

See: [Results tables](RESULTS.md)

### Operations

  1. Axpy (a.k.a. TRIAD):
        Multiplies each element of one vector by a scalar and adds it to another vector.

            d[i] = alpha * a[i] + b[i];

  2. Copy:
      Copies elements from one vector to another.

            d[i] = a[i];

  3. FMA (Fused Multiply-Add):
        Performs a multiply-add operation on vectors.

            d[i] = a[i] * b[i] + c[i];
        
  4. Add Mult (In this benchmark the amount of input and output data are equal):
        adds and multiply two vectors,

            d[i] = a[i] + b[i];
            c[i] = a[i] * b[i];

### Applications

My stream consists of three application which perform the memory bandwidth benchmark in three different conditions:

* **my_stream_OMP** The original stream benchmark approach based on OpenMP.

* **my_stream_mt_gm** It executes a number of threads that is equal to the number of CPU and performs the operation on different portions of pages of memory allocated globally by the main thread.

* **my_stream_mt_lm**  It executes a number of threads that is equal to the number of CPU and performs the operation on pages of memory allocated locally in the threads.

* **my_stream_MPI** Test based on MPI. This is the test which allows the maximal bandwidth.

### Usage

##### Clone and compile:

      git clone https://github.com/simon-r/My_Stream_Benchmark.git
      cd My_Stream_Benchmark
      make

##### Execute:

      ./my_stream_OMP.exe -s {vec_size}
      ./my_stream_mt_gm.exe -s {vec_size}
      ./my_stream_mt_lm.exe -s {vec_size}
      mpirun -n #NR_CPU ./my_stream_MPI.exe -s {vec_size}

For a reliable measurement, make sure that the total allocated memory is approximately half of the total available DRAM.


### Benchmarking

* Threads: It uses pthreads for parallel execution.
* Semaphores: Employed for synchronization among threads.
* Timing: Uses high-resolution clocks to measure execution time.
* Repetitions: Operations are repeated multiple times (controlled by BENCHMARK_REPETITIONS) to calculate average time.

### Workflow

Allocates and initializes vectors (a, b, c, d) with random data.
Divides vectors into batches for parallel processing.
Executes each operation (axpy, copy, fma) multiple times and calculates average execution times.
Computes memory consumption and bandwidth for each operation.

### Output

Provides detailed information regarding vector sizes, CPU count, adjusted vector sizes, memory sizes, and repetitions.
Reports average execution times, bandwidth and memory used for
Axpy, Copy, and FMA operations.

### Tested compilers
* GCC 7.5.0, 13.2.1 14.2.1
* CLANG 16.0.6 18.1.8
* icx 2023.2.0, 2023.0.0

The support of c99 is required.

### Change compiler
      make CC={your_favorite_CC_compiler_cmd}

### Notes

The program includes extensive comments documenting various functions, data structures, and calculations.
The VERBOSE macro can be used to enable/disable detailed output.



### Licensing

The program is licensed under the GNU General Public License, allowing redistribution and modification under specific terms outlined by the Free Software Foundation.

It's a thorough piece of code designed to assess the performance of vector operations in terms of speed and memory usage.
