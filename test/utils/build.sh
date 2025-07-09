#!/bin/sh

WUK_INSTALL_PREFIX=$1

rm -rf $WUK_INSTALL_PREFIX
if [ $CUDACXX != "nvcc" ]; then
    WUK_CUDA_FLAGS=" -D__STRICT_ANSI__ "
else
    WUK_CUDA_FLAGS=" -D__STRICT_ANSI__ -allow-unsupported-compiler "
fi
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CUDA_FLAGS="$WUK_CUDA_FLAGS" \
    -DCMAKE_INSTALL_PREFIX=$WUK_INSTALL_PREFIX \
    -DANTLR4_JAR_LOCATION=$ANTLR4_JAR_LOCATION \
    -DGOPTX_BUILD_TEST=ON \
    -S $(realpath $(dirname $0)/../..) \
    -B $WUK_INSTALL_PREFIX/build
cmake --build $WUK_INSTALL_PREFIX/build -j
cmake --build $WUK_INSTALL_PREFIX/build -t install
