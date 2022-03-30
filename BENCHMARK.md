# Benchmark Results

## SETI-NODE8 (AMD EPYC 7352 24-Core @ 48x 2.3GHz)

### -O0
```
CPU Caches:
  L1 Data 32K (x48)
  L1 Instruction 32K (x48)
  L2 Unified 512K (x48)
  L3 Unified 16384K (x16)
Load Average: 2.27, 3.57, 4.07
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
STANDARD/iterations:1073741824       70.6 ns         70.6 ns   1073741824
```

### -O3
```
CPU Caches:
  L1 Data 32K (x48)
  L1 Instruction 32K (x48)
  L2 Unified 512K (x48)
  L3 Unified 16384K (x16)
Load Average: 1.13, 2.33, 3.46
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
STANDARD/iterations:1073741824       41.0 ns         41.0 ns   1073741824
```

## Laptop (Apple M1 Pro)

### -O0
```
CPU Caches:
  L1 Data 64 KiB (x8)
  L1 Instruction 128 KiB (x8)
  L2 Unified 4096 KiB (x4)
Load Average: 2.21, 2.01, 1.90
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
STANDARD/iterations:1073741824       28.7 ns         28.7 ns   1073741824
```

### -O3
```
CPU Caches:
  L1 Data 64 KiB (x8)
  L1 Instruction 128 KiB (x8)
  L2 Unified 4096 KiB (x4)
Load Average: 2.87, 3.12, 2.60
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
STANDARD/iterations:1073741824       14.8 ns         14.8 ns   1073741824
```