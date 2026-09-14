# ⚡CUDACyclone: GPU Satoshi Puzzle Solver

Cyclone CUDA is the GPU-powered version of the **Cyclone** project, designed to achieve extreme performance in solving Satoshi puzzles on modern NVIDIA GPUs.  
Leveraging **CUDA**, **warp-level parallelism**, and **batch EC operations**, Cyclone CUDA pushes the limits of cryptographic key search.

Secp256k1 math is based on the excellent work from [JeanLucPons/VanitySearch](https://github.com/JeanLucPons/VanitySearch), and [FixedPaul/VanitySearch-Bitcrack](https://github.com/FixedPaul) with major CUDA-specific modifications.  
Special thanks to Jean-Luc Pons for his foundational contributions to the cryptographic community.

Cyclone CUDA also is the **simplest CUDA-based project** for solving Satoshi puzzles on GPU.  
It was designed with clarity and minimalism in mind — making it easy to **compile, understand, and run**, even for those new to CUDA programming.  

Despite its simplicity, Cyclone CUDA leverages **massive GPU parallelism** to achieve extreme performance in elliptic curve computations and Hash160 pipelines. 

⚠️ **Achieved 6.5Gkeys/s on RTX4090. 8.6Gkeys/s on RTX5090.**  
⚠️ **For preventing decresasing GPU speed you need to use --slices option and not to start brooteforce with 256 points per batch! THe best tune for 4090 is --grid 128,128 --slices 16**

---

## ⚠️ Problem with a key skipping was fixed!
My software was skipping keys earlier.  
I fixed this problem, to make sure that keys are not skipped I wrote a small script in python.   
The script is called **proof.py**. The script generates random scalars in a given range, calculates addresses, then runs Cyclone and at the end of the work shows how many keys were found and how many were not found. All should be found.
Usage
```
sage: proof.py [-h] --range RANGE_ARG [--cyclone-path CYCLONE_PATH] [--grid GRID_ARG] [--batch BATCH] [--timeout TIMEOUT] [--start-count START_COUNT] [--end-count END_COUNT] [--quartile-count QUARTILE_COUNT]
```
Sample start
```
python3 proof.py --range 200000000:3FFFFFFFF --grid 512,512
```
Results
```
================ Summary by blocks ================
Range start A (start+2k)           : total= 128  success= 128  fail=   0
Range start B (start+1+2k)         : total= 128  success= 128  fail=   0
Range end A (end-2k)               : total= 128  success= 128  fail=   0
Range end B (end-1-2k)             : total= 128  success= 128  fail=   0
Full mod 512 residue coverage      : total= 256  success= 256  fail=   0
Random Q1 (0–25%)                  : total=  20  success=  20  fail=   0
Random Q2 (25–50%)                 : total=  20  success=  20  fail=   0
Random Q3 (50–75%)                 : total=  20  success=  20  fail=   0
Random Q4 (75–100%)                : total=  20  success=  20  fail=   0

Done. Results in cyclone_tests_results.txt. Successes=848 Failures=0
```

After speed upgrade on RTX 4060 - tests
```
================ Summary by blocks ================
Range start A (start+2k)           : total= 128  success= 128  fail=   0
Range start B (start+1+2k)         : total= 128  success= 128  fail=   0
Range end A (end-2k)               : total= 128  success= 128  fail=   0
Range end B (end-1-2k)             : total= 128  success= 128  fail=   0
Full mod 512 residue coverage      : total= 256  success= 256  fail=   0
Random Q1 (0–25%)                  : total=  20  success=  20  fail=   0
Random Q2 (25–50%)                 : total=  20  success=  20  fail=   0
Random Q3 (50–75%)                 : total=  20  success=  20  fail=   0
Random Q4 (75–100%)                : total=  20  success=  20  fail=   0

Done. Results in cyclone_tests_results.txt. Successes=848 Failures=0

```
## 🚀 Key Features

- **GPU Acceleration**: Optimized for NVIDIA GPUs with full CUDA support.
- **Massive Parallelism**: Tens of thousands of threads computing elliptic curve points and **hash160** simultaneously.
- **Batch EC Operations**: Efficient group addition and modular inversion with warp-level optimizations.
- **Grid/Batch Control**: Fully configurable GPU execution with `--grid` parameter (threads per batch × points per batch).
- **Cross-Platform**: Works on Linux and Windows (via WSL2 or MinGW cross-compilation).
- **Cross Architecture**: Automatic compilation for different architectures (75 86 89).
- **Extremely low VRAM usage**: Key feature! For low price rented GPU.
---

## 🚀 Options
- **--range**: range of search. Must be a power of two!
- **--address**: P2PKH address.
- **--target-hash160**: the same as address but hash160.
- **--grid**: very usefull parameter. Example --grid 512,512 - first 512 - number of points each thread will process in one batch (Points batch size)., second 512 - number of threads in one group (Threads per batch).
- **--slices**: batch per thread for one kernel launch.

---

### ❔ Community benchmarks

Users have reported the following speeds:

| GPU               | Grid      | Speed (Mkeys/s) | Notes                  |
|-------------------|-----------|-----------------|------------------------|
| RTX 4090          | 128,1024  | 6214 Mkeys/s    | Community report       |
| RTX 4090          | 512,512   | 6038 Mkeys/s    | Community report       |
| RTX 4060          | 512,512   | 1238 Mkeys/s    | My own GPU             |
| RTX 4070 Ti Super | 512,1024  | 3170 Mkeys/s    | Community report       |
| L4-2Q             | 512,256   | 1360 Mkeys/s    | Community report       |
| RTX3070 mobile    | 256,256   | 1150 Mkeys/s    | Community report       |

---

## 🔷 Example Output

Below is an example run of **Cyclone CUDA**.  

**RTX4060**

```bash
./CUDACyclone --range 2000000000:3FFFFFFFFF --address 1HBtApAFA9B2YZw3G2YKSMCtb3dVnjuNe2 --grid 512,256
======== PrePhase: GPU Information ====================
Device               : NVIDIA GeForce RTX 4060 (compute 8.9)
SM                   : 24
ThreadsPerBlock      : 256
Blocks               : 4096
Points batch size    : 512
Batches/SM           : 256
Memory utilization   : 6.9% (538.3 MB / 7.63 GB)
------------------------------------------------------- 
Total threads        : 1048576

======== Phase-1: Brooteforce =========================
Time: 8.0 s | Speed: 1268.9 Mkeys/s | Count: 10204470016 | Progress: 7.42 %

======== FOUND MATCH! =================================
Private Key   : 00000000000000000000000000000000000000000000000000000022382FACD0
Public Key    : 03C060E1E3771CBECCB38E119C2414702F3F5181A89652538851D2E3886BDD70C6
```

**RTX4090**
```bash
./CUDACyclone --range 200000000000:3fffffffffff --address 1F3JRMWudBaj48EhwcHDdpeuy2jwACNxjP --grid 128,128 --slices 16
======== PrePhase: GPU Information ====================
Device               : NVIDIA GeForce RTX 4090 (compute 8.9)
SM                   : 128
ThreadsPerBlock      : 256
Blocks               : 16384
Points batch size    : 128
Batches/SM           : 128
Batches/launch       : 16 (per thread)
Memory utilization   : 4.8% (1.14 GB / 23.6 GB)
-------------------------------------------------------
Total threads        : 4194304

======== Phase-1: BruteForce (sliced) =================
Time: 393.7 s | Speed: 6127.4 Mkeys/s | Count: 2421341587872 | Progress: 6.88 %

======== FOUND MATCH! =================================
Private Key   : 00000000000000000000000000000000000000000000000000002EC18388D544
Public Key    : 03FD5487722D2576CB6D7081426B66A3E2986C1CE8358D479063FB5F2BB6DD5849
```
**RTX5090**
```bash
./CUDACyclone --range 200000000000:3fffffffffff --address 1F3JRMWudBaj48EhwcHDdpeuy2jwACNxjP —-grid 128,256
======== PrePhase: GPU Information ====================
Device               : NVIDIA GeForce RTX 5090 (compute 12.0)
SM                   : 170
ThreadsPerBlock      : 256
Blocks               : 1024
Points batch size    : 128
Batches/SM           : 8
Memory utilization   : 1.7% (557.3 MB / 31.4 GB)
------------------------------------------------------- 
Total threads        : 262144

======== Phase-1: Brooteforce =========================
Time: 7.0 s | Speed: 8408.0 Mkeys/s | Count: 58545467200 | Progress: 0.17 %

```
**RTX3070 mobile**
```bash
./CUDACyclone --range 2000000000:3FFFFFFFFF --address 1HBtApAFA9B2YZw3G2YKSMCtb3dVnjuNe2 --grid 512,256
======== PrePhase: GPU Information ====================
Device               : NVIDIA GeForce RTX 3070 Laptop GPU (compute 8.6)
SM                   : 40
ThreadsPerBlock      : 256
Blocks               : 8192
Points batch size    : 512
Batches/SM           : 256
Batches/launch       : 64 (per thread)
Memory utilization   : 64.0% (5.12 GB / 8.00 GB)
-------------------------------------------------------
Total threads        : 2097152

======== Phase-1: BruteForce (sliced) =================
Time: 61.2 s | Speed: 1234.3 Mkeys/s | Count: 72707573152 | Progress: 52.90 %

======== FOUND MATCH! =================================
Private Key   : 00000000000000000000000000000000000000000000000000000022382FACD0
Public Key    : 03C060E1E3771CBECCB38E119C2414702F3F5181A89652538851D2E3886BDD70C6
```
## 🛠️ Getting Started
To get started with CUDACyclone, clone the repository and type **make**  
For totaly clean system (big thanks for **dev_nullish**):
```bash
apt update;
apt-get install -y joe;
apt-get install -y zip;
apt-get install -y screen;
apt-get install -y curl libcurl4;
apt-get install build-essential;
apt-get install -y gcc;
apt-get install -y make;
apt install cuda-toolkit;
git clone https://github.com/Dookoo2/CUDACyclone.git
make
```

### Building on Windows
MSVC, the host compiler of `nvcc` on Windows, has no `__int128`. Under `#if defined(_WIN32)` the affected
functions in `CUDAMath.h` and `CUDAUtils.h` use 64-bit limbs with an explicit carry instead; the Linux code
is unchanged. None of these functions is in the hot kernel loop, so speed is not affected.

Easiest way: the GitHub Actions workflow `.github/workflows/build.yml` builds `CUDACyclone.exe` on
`windows-2022` with CUDA 12.9.1 (Actions → *Build CUDACyclone (Windows, CUDA 12.9)* → artifact).

Manual build: install Visual Studio 2022 (C++ workload) and the CUDA Toolkit 12.x, open a
*x64 Native Tools* prompt and run the same commands as the workflow. Pick the `-gencode` entries for your
GPU: 75 = RTX 20, 86 = RTX 30, 89 = RTX 40, 120 = RTX 50.
```bat
set FLAGS=-O3 -rdc=true -use_fast_math --ptxas-options=-O3 -gencode arch=compute_86,code=sm_86 -std=c++17
nvcc %FLAGS% -c CUDACyclone.cu -o CUDACyclone.obj
nvcc %FLAGS% -c CUDAHash.cu -o CUDAHash.obj
nvcc %FLAGS% CUDACyclone.obj CUDAHash.obj -o CUDACyclone.exe -lcudadevrt -cudart=static
```
Tested with an RTX 5060 Ti (CUDA 12.9.1, driver on Windows 11).

## 🚧**Version**
**V1.3**: Full CUDA Kernel rewrite again for preventing key skipping.    
**V1.2**: Full CUDA Kernel rewrite.  
**V1.1**: Switch pGx/pGy to constant memory due to VRAM thermal throttling.  
**V1.0**: Release.



## ✌️**TIPS**
BTC: bc1qtq4y9l9ajeyxq05ynq09z8p52xdmk4hqky9c8n
