# The LLVM Compiler Infrastructure

[![OpenSSF Scorecard](https://api.securityscorecards.dev/projects/github.com/llvm/llvm-project/badge)](https://securityscorecards.dev/viewer/?uri=github.com/llvm/llvm-project)
[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/8273/badge)](https://www.bestpractices.dev/projects/8273)
[![libc++](https://github.com/llvm/llvm-project/actions/workflows/libcxx-build-and-test.yaml/badge.svg?branch=main&event=schedule)](https://github.com/llvm/llvm-project/actions/workflows/libcxx-build-and-test.yaml?query=event%3Aschedule)

Welcome to the LLVM project!

This repository contains the source code for LLVM, a toolkit for the
construction of highly optimized compilers, optimizers, and run-time
environments.

The LLVM project has multiple components. The core of the project is
itself called "LLVM". This contains all of the tools, libraries, and header
files needed to process intermediate representations and convert them into
object files. Tools include an assembler, disassembler, bitcode analyzer, and
bitcode optimizer.

C-like languages use the [Clang](https://clang.llvm.org/) frontend. This
component compiles C, C++, Objective-C, and Objective-C++ code into LLVM bitcode
-- and from there into object files, using LLVM.

Other components include:
the [libc++ C++ standard library](https://libcxx.llvm.org),
the [LLD linker](https://lld.llvm.org), and more.

## Getting the Source Code and Building LLVM

Consult the
[Getting Started with LLVM](https://llvm.org/docs/GettingStarted.html#getting-the-source-code-and-building-llvm)
page for information on building and running LLVM.

For information on how to contribute to the LLVM project, please take a look at
the [Contributing to LLVM](https://llvm.org/docs/Contributing.html) guide.

## Setup instructions for this fork
1. clone the repository
2. install the repository using the same method as a typical MLIR installation. Instructions can be found within this [link](https://mlir.llvm.org/getting_started/)
3. ensure that you have ninja
4. run ninja mlir-opt in the build directory

## Conversion instructions 
note: this fork is only meant to be a proof of concept. Calls other than __syncwarp still remains untranslated and hence uncompilable to sm_60 if they are present 
to convert the cuda kernel to .ll: 
```
clang++ -S -emit-llvm --cuda-gpu-arch=sm_70 -xcuda sync.cu
```
to convert the .ll to mlir:
```
mlir-opt sync.mlir --convert-syncwarp-to-pascal -o sync_pascal.mlir
```
to translate the mlir:
```
mlir-translate --mlir-to-llvmir sync_pascal.mlir -o sync_pascal.ll
```
convert back to ptx kernel:
```
llc -march=nvptx64 -mcpu=sm_60 sync_pascal.ll -o sync_sm60.ptx
```





