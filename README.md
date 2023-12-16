# My Stream Benchmark
-------------------------

This program, "my_stream.c," is designed to benchmark the memory bandwidth (in Mb/s and Gb/s).
It evaluates three "memory bound" vector operations: Axpy, Copy, and FMA (fused multiply-add).

### Key Components:

* Libraries Included: OpenMP, pthread, semaphore, stdio, stdlib, time.
* Data Types: It uses a typedef float_type as double.
* Vector Operations: Utilizes SIMD (Single Instruction, Multiple Data) capabilities to process vectors efficiently.

### Operations

  1. Axpy (Vector operation):
        Multiplies each element of one vector by a scalar and adds it to another vector.
        d[i] = aa * a[i] + b[i];

  2. Copy:
        Copies elements from one vector to another.
        d[i] = a[i];

  3. FMA (Fused Multiply-Add):
        Performs a multiply-add operation on vectors.
        d[i] = a[i] * b[i] + c[i];

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

Provides detailed information on vector sizes, CPU count, corrected
vector sizes, memory sizes, and repetitions.
Outputs average execution times, bandwidth, and memory streamed for
Axpy, Copy, and FMA operations.

### Notes

The program includes extensive comments documenting various functions, data structures, and calculations.
The VERBOSE macro can be used to enable/disable detailed output.

### Licensing

The program is licensed under the GNU General Public License, allowing redistribution and modification under specific terms outlined by the Free Software Foundation.

It's a thorough piece of code designed to assess the performance of vector operations in terms of speed and memory usage.
