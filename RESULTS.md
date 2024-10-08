# Results


## Table of Contents

- [Intel(R) Xeon(R) CPU E5-2695 v4 @ 2.10GHz (2x NUMA CPUs)](#intelr-xeonr-cpu-e5-2695-v4--210ghz-2x-numa-cpus)
    - [Multi Threads - Local Memory](#multi-threads---local-memory)
    - [Multi Threads - Global Memory](#multi-threads---global-memory)
    - [OpenMP](#openmp)
    - [MPI](#mpi)
- [Intel(R) Xeon(R) Platinum 8480+ (2x NUMA CPUs)](#intelr-xeonr-platinum-8480-2x-numa-cpus)
- [NVIDIA Grace CPU [4x NUMA CPUs]](#nvidia-grace-cpu-4x-numa-cpus)
- [AMD EPYC 7742 64-Core (2x NUMA CPUs)](#amd-epyc-7742-64-core-2x-numa-cpus)

## Intel(R) Xeon(R) CPU E5-2695 v4 @ 2.10GHz (2x NUMA CPUs)

Compiler gcc 13.2

    ------------------------------------------------------------------
    Number of MPI processes:               36
    Adjusted vector size:                  500000256
    MB Vector size per process:            105.963867
    GB Vector size per process:            0.103480
    GB Total allocated memory:             14.901169
    Repetitions:                           50
    ------------------------------------------------------------------


#### Multi Threads - Local Memory

    Test       bandwidth     mean time
    -----------------------------------------------------------
    AXPY:      89.314 GB/s   125.130003 ms
    Copy:      102.265 GB/s   72.855763 ms
    FMA:       96.156 GB/s   154.968748 ms
    Add Mul:   77.379 GB/s   192.574447 ms
    -----------------------------------------------------------

#### Multi Threads - Global Memory

    Benchmark:             MB/s                     GB/s          Memory Streamed [MB]
    -----------------------------------------------------------------------------------------------

    Axpy:                  44830.63                 43.78         572204.882812
    Copy:                  38845.40                 37.93         381469.921875
    FMA:                   49468.75                 48.31         762939.843750
    Add Mult:              38851.89                 37.94         762939.843750
    -----------------------------------------------------------------------------------------------

#### OpenMP

    -----------------------------------------------------------
    FMA:                       62.593331 [GB/s]
    Clock:                     238.063204 [ms]
    Consume:  510.388650
    -----------------------------------------------------------
    AXPY:                      81.027703 [GB/s]
    Clock:                     137.926614 [ms]
    Consume:  568.710133
    -----------------------------------------------------------
    Copy:                      68.277651 [GB/s]
    Clock:                     109.121861 [ms]
    Consume:  179.885000
    -----------------------------------------------------------
    Add Mul:                   69.198126 [GB/s]
    Clock:                     215.340641 [ms]
    Consume:  504.003517
    -----------------------------------------------------------

#### MPI

    Results:
    ------------------------------------------------------------------
    Test            Total bandwidth        clock  
    ------------------------------------------------------------------
    FMA:            96.162 GB/s,          154.958954 ms
    copy:           77.436 GB/s,          96.221861 ms
    axpy (TRIAD):   89.076 GB/s,          125.466908 ms
    add mul:        77.362 GB/s,          192.629216 ms
    ------------------------------------------------------------------

## Intel(R) Xeon(R) Platinum 8480+ (2x NUMA CPUs)

    ------------------------------------------------------------------
    Start My Stream [MPI]
    ------------------------------------------------------------------

    User defined vector size: 1000000000

    ------------------------------------------------------------------
    Number of MPI processes:               112
    Adjusted vector size:                  1000000512
    MB Vector size per process:            68.119629
    GB Vector size per process:            0.066523
    GB Total allocated memory:             29.802338
    Repetitions:                           50
    ------------------------------------------------------------------
    ------------------------------------------------------------------
    Test            Total bandwidth        clock  
    ------------------------------------------------------------------
    FMA:            379.202 GB/s,          78.594036 ms
    copy:           354.011 GB/s,          42.092840 ms
    axpy (TRIAD):   372.228 GB/s,          60.048817 ms
    add mul:        318.014 GB/s,          93.714378 ms

    ------------------------------------------------------------------

## NVIDIA Grace CPU [4x NUMA CPUs]

    ------------------------------------------------------------------
    Start My Stream [MPI]
    ------------------------------------------------------------------
    
    User defined vector size: 3000000000
    
    ------------------------------------------------------------------
    Number of MPI processes:               288
    Adjusted vector size:                  3000001536
    MB Vector size per process:            79.472900
    GB Vector size per process:            0.077610
    GB Total allocated memory:             89.407013
    Repetitions:                           50
    ------------------------------------------------------------------
    
    Results:
    ------------------------------------------------------------------
    Test            Total bandwidth        clock  
    ------------------------------------------------------------------
    FMA:            1873.619 GB/s,          47.723450 ms
    copy:           1982.381 GB/s,          22.559055 ms
    axpy (TRIAD):   1882.151 GB/s,          35.631114 ms
    add mul:        1915.613 GB/s,          46.683713 ms
    
    ------------------------------------------------------------------

## AMD EPYC 7742 64-Core (2x NUMA CPUs)


    ------------------------------------------------------------------
    Start My Stream [MPI]
    ------------------------------------------------------------------
    
    User defined vector size: 1000000000
    
    ------------------------------------------------------------------
    Number of MPI processes:               128
    Adjusted vector size:                  1000000512
    MB Vector size per process:            59.604675
    GB Vector size per process:            0.058208
    GB Total allocated memory:             29.802338
    Repetitions:                           50
    ------------------------------------------------------------------
    
    Results:
    ------------------------------------------------------------------
    Test            Total bandwidth        clock  
    ------------------------------------------------------------------
    FMA:            219.307 GB/s,          135.896766 ms
    copy:           276.344 GB/s,          53.924325 ms
    axpy (TRIAD):   206.751 GB/s,          108.111409 ms
    add mul:        181.459 GB/s,          164.240278 ms
    
    ------------------------------------------------------------------



