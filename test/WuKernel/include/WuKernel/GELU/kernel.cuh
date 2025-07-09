#pragma once
#include <cstdint>
#include <cuda_fp16.h>

namespace wuk {

namespace GELU {

// https://github.com/microsoft/onnxruntime/blob/v1.20.0/onnxruntime/contrib_ops/cuda/activation/activations_impl.cu#L40

template <typename T> struct OP_QuickGelu {
  __device__ __forceinline__ T operator()(const T &a) {
    T alpha = static_cast<T>(1.702f);
    T v = a * static_cast<T>(alpha);
    T one = static_cast<T>(1.f);
    T zero = static_cast<T>(0.f);
    T sigmoid = one / (one + exp(-v));
    return a * sigmoid;
  }
};

template <typename T = __half, typename Tstore = __half2,
          typename T_op = OP_QuickGelu<float>>
__global__ void quickGELU(size_t n, T *x) {
  for (size_t i = size_t(sizeof(Tstore) / sizeof(T)) *
                  (threadIdx.x + blockIdx.x * size_t(blockDim.x));
       i < n;
       i += size_t(sizeof(Tstore) / sizeof(T)) * blockDim.x * gridDim.x) {
    union {
      Tstore store;
      T value[(sizeof(Tstore) / sizeof(T))];
    } simd;
    auto p = reinterpret_cast<Tstore *>(&x[i]);
    simd.store = *p;
    T_op op;
#pragma unroll
    for (int j = 0; j < (sizeof(Tstore) / sizeof(T)); ++j)
      simd.value[j] = op(simd.value[j]);
    *p = simd.store;
  }
}

#define WUK_DEFINE_KERNEL_GELU                                                 \
  template __global__ void quickGELU<>(size_t n, __half * x)

extern WUK_DEFINE_KERNEL_GELU;

}; // namespace GELU

}; // namespace wuk