#pragma once
#include <cstdint>

namespace wuk {

namespace SORT {

// https://github.com/accel-sim/gpu-app-collection/blob/v1.1/src/cuda/rodinia/3.1/cuda/hybridsort/bucketsort_kernel.cu#L82

template <int LOG_DIVISIONS = 5>
__global__ void bucketsort(float *input, unsigned int *indice, float *output,
                           int size, unsigned int *d_prefixoffsets,
                           unsigned int *l_offsets) {
  const auto BUCKET_WARP_LOG_SIZE = 5, DIVISIONS = 1 << LOG_DIVISIONS;
  const auto BUCKET_WARP_N = 8;
  const auto BUCKET_BLOCK_MEMORY = DIVISIONS * BUCKET_WARP_N;
  volatile __shared__ unsigned int s_offset[BUCKET_BLOCK_MEMORY];

  int prefixBase = blockIdx.x * BUCKET_BLOCK_MEMORY;
  const int warpBase = (threadIdx.x >> BUCKET_WARP_LOG_SIZE) * DIVISIONS;
  const int numThreads = blockDim.x * gridDim.x;
  for (int i = threadIdx.x; i < BUCKET_BLOCK_MEMORY; i += blockDim.x)
    s_offset[i] =
        l_offsets[i & (DIVISIONS - 1)] + d_prefixoffsets[prefixBase + i];

  __syncthreads();

  for (int tid = blockIdx.x * blockDim.x + threadIdx.x; tid < size;
       tid += numThreads) {

    float elem = input[tid];
    auto id = indice[tid];

    auto test =
        s_offset[warpBase + (id & (DIVISIONS - 1))] + (id >> LOG_DIVISIONS);

#if 0 // for test with random input
    output[test] = elem;
#else
    output[tid] = elem * test;
#endif
  }
}

#define WUK_DEFINE_KERNEL_SORT                                                 \
  template __global__ void bucketsort<>(                                       \
      float *input, unsigned int *indice, float *output, int size,             \
      unsigned int *d_prefixoffsets, unsigned int *l_offsets)

extern WUK_DEFINE_KERNEL_SORT;

} // namespace SORT

} // namespace wuk