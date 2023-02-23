# CATO

CATO (Compiler Assisted Source Transformation of OpenMP Kernels) uses LLVM and Clang to transform existing OpenMP code to MPI. This enables distributed code execution while keeping OpenMP's relatively low barrier of entry. The main focus lies on increasing the maximum problem size, which a scientific application can work on. Converting an intra-node problem into an inter-node problem makes it possible to overcome the limitation of memory of a single node.


## Using CATO

### Dependencies

| CATO |  LLVM  | MPICH | netcdf-c |
|:----:|:------:|:-----:|:--------:|
|  0.1 | 12.0.0 | 3.3.1 |     x    |
|  0.2 | 13.0.0 | 3.3.1 |   4.8.1  |

It is important to pay attention to a right match of CATO and LLVM. Major LLVM releases tend to induce ABI breaking changes.

### Building the LLVM Pass
CATO is an LLVM pass, which is applied during the optimisation phase.

```
$ scripts/build_pass.sh
```

To make sure to remove old existing files, add `--rebuild` flag to `build_pass.sh` to delete old build directory (important if you switch LLVM versions).

### Using the LLVM Pass

```
$ scripts/cexecute_pass.py inputfile.c -o inputfile_modified.x
```

The pass can also be applied manually, which makes it easier to follow single steps. Those steps could be shortened but the long version is used here to get more intermediate results (useful for debugging).

1. Create IR code from `inputfile.c`
```shell
$ clang emit-llvm -S inputfile.c
```

2. Apply CATO LLVM pass to get modified IR
```shell
$ opt -load-pass-plugin=libCatoPass.so -passes=Cato inputfile.ll -S -o inputfile_modified.ll
```

3. Create modified LLVM bytecode from IR code
```shell
$ llvm-as inputfile_modified.ll -o inputfile_modified.bc
```

4. Create final binary from modified LLVM bytecode
```shell
mpicc -cc=clang -o inputfile_modified_binary.x inputfile_modified.ll libCatoRuntime.so
```

You need to adjust the path to `libCatoPass.so` and `libCatoRuntime.so`. The generated binary file can now be executed with `mpiexec`.


# Citing CATO
If you are referencing CATO in a publication, please cite the following paper:


J. Squar, T. Jammer, M. Blesel, M. Kuhn and T. Ludwig, "Compiler Assisted Source Transformation of OpenMP Kernels," 2020 19th International Symposium on Parallel and Distributed Computing (ISPDC), Warsaw, Poland, 2020, pp. 44-51, doi: 10.1109/ISPDC51135.2020.00016.

