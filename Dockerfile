# syntax=docker/dockerfile:1.4
FROM nvidia/cuda:12.6.3-devel-ubuntu24.04 as devel
ENV PATH=/root/GoPTX_nvcc/bin:$PATH \
  CPATH=/root/GoPTX_nvcc/include:$CPATH \
  LIBRARY_PATH=/root/GoPTX_nvcc/lib:$LIBRARY_PATH \
  LD_LIBRARY_PATH=/root/GoPTX_nvcc/lib:$LD_LIBRARY_PATH \
  ANTLR4_JAR_LOCATION=/root/antlr-4.10.1-complete.jar
COPY . /root/GoPTX
RUN <<EOF

apt-get update -y
apt-get upgrade -y
apt-get install --no-install-recommends -y \
  wget cmake clang-19 ninja-build \
  libantlr4-runtime-dev default-jre-headless pkg-config uuid-dev

apt-get autoremove -y
apt-get clean -y
rm -rf /var/lib/apt/lists/*

wget https://www.antlr.org/download/antlr-4.10.1-complete.jar -O $ANTLR4_JAR_LOCATION

CC=clang-19 CXX=clang++-19 CUDAHOSTCXX=clang++-19 CUDACXX=nvcc CUDAARCHS="80;90" /root/GoPTX/test/utils/build.sh /root/GoPTX_nvcc
cp -r /root/GoPTX/test/utils /root/GoPTX_nvcc/
rm -rf /root/GoPTX_nvcc/build /root/GoPTX_nvcc/lib/*.a /root/GoPTX_nvcc/include /root/GoPTX_nvcc/utils/*.sh

CC=clang-19 CXX=clang++-19 CUDAHOSTCXX=clang++-19 CUDACXX=clang++-19 CUDAARCHS="80;90" /root/GoPTX/test/utils/build.sh /root/GoPTX_clang
cp -r /root/GoPTX/test/utils /root/GoPTX_clang/
rm -rf /root/GoPTX_clang/build /root/GoPTX_clang/lib/*.a /root/GoPTX_clang/include /root/GoPTX_clang/utils/*.sh

cd /root/GoPTX_nvcc/utils
echo "__global__ void namea(int *a) { *a += 1; }" > a.cu
nvcc -ptx a.cu
echo "__global__ void nameb(float *b) { *b += 1; }" > b.cu
nvcc -ptx b.cu
GoPTX_link a.ptx b.ptx _sa _sb > c.ptx
GoPTX_merge c.ptx \$1_\$2_merged namea nameb > d.ptx
cat d.ptx
EOF
FROM nvidia/cuda:12.6.3-base-ubuntu24.04 as base
ENV PATH=/root/GoPTX_nvcc/bin:$PATH \
  CPATH=/root/GoPTX_nvcc/include:$CPATH \
  LIBRARY_PATH=/root/GoPTX_nvcc/lib:$LIBRARY_PATH \
  LD_LIBRARY_PATH=/root/GoPTX_nvcc/lib:$LD_LIBRARY_PATH
RUN <<EOF
apt-get update -y
apt-get upgrade -y
DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
  libantlr4-runtime-dev python3-altair

apt-get autoremove -y
apt-get clean -y
rm -rf /var/lib/apt/lists/*
EOF
COPY --from=devel /root/GoPTX_nvcc /root/GoPTX_nvcc
COPY --from=devel /root/GoPTX_clang /root/GoPTX_clang
CMD ["/bin/sh", "-c", "echo '[GoPTX]: start testing...' && cd /root/GoPTX_nvcc/utils/results && GoPTX_test > results.html && python3 ../analyse.py < results.html"]
