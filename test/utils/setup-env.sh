#!/bin/sh

spack load --first cmake@3.27:
spack load --first ninja
spack load --first antlr4-cpp-runtime@4.11:
spack load --first antlr4-complete@4.11:
spack load --first py-altair
spack load --first py-vl-convert-python
spack load --first llvm+clang targets=all
spack load --first cuda@12.6

spack find --loaded
