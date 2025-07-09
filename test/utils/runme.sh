#!/bin/sh

. $SCC_SETUP_ENV

. $(realpath $(dirname $0))/setup-env.sh

CC=clang CXX=clang++ CUDAHOSTCXX=clang++ CUDACXX=nvcc CUDAARCHS="80;90" $(realpath $(dirname $0))/build.sh $HOME/GoPTX_nvcc

$(realpath $(dirname $0))/test.sh $HOME/GoPTX_nvcc

(export WUK_UTIL_PATH=$(realpath $(dirname $0)) && cd $HOME/GoPTX_nvcc/build/test && python3 $WUK_UTIL_PATH/analyse.py <results.html)

CC=clang CXX=clang++ CUDAHOSTCXX=clang++ CUDACXX=clang++ CUDAARCHS="80;90" $(realpath $(dirname $0))/build.sh $HOME/GoPTX_clang++

$(realpath $(dirname $0))/test.sh $HOME/GoPTX_clang++

(export WUK_UTIL_PATH=$(realpath $(dirname $0)) && cd $HOME/GoPTX_clang++/build/test && python3 $WUK_UTIL_PATH/analyse.py <results.html)
