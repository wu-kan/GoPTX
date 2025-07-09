#pragma once
#include <cstdint>

namespace wuk {

namespace MICND {

// https://github.com/accel-sim/gpu-app-collection/blob/v1.1/src/cuda/NVIDIA_CUDA-11.0_Samples/quasirandomGenerator/quasirandomGenerator_kernel.cu

////////////////////////////////////////////////////////////////////////////////
// Moro's Inverse Cumulative Normal Distribution function approximation
////////////////////////////////////////////////////////////////////////////////
template <typename T = float>
__global__ void MoroInvCNDgpu(T *d_Output, unsigned int pathN) {
  const unsigned int pos = blockDim.x * blockIdx.x + threadIdx.x;
  if (pos >= pathN)
    return;
  const unsigned int distance = ((unsigned int)-1) / (pathN + 1);
  unsigned int x = (pos + 1) * distance;

  const T a1 = 2.50662823884f;
  const T a2 = -18.61500062529f;
  const T a3 = 41.39119773534f;
  const T a4 = -25.44106049637f;
  const T b1 = -8.4735109309f;
  const T b2 = 23.08336743743f;
  const T b3 = -21.06224101826f;
  const T b4 = 3.13082909833f;
  const T c1 = 0.337475482272615f;
  const T c2 = 0.976169019091719f;
  const T c3 = 0.160797971491821f;
  const T c4 = 2.76438810333863E-02f;
  const T c5 = 3.8405729373609E-03f;
  const T c6 = 3.951896511919E-04f;
  const T c7 = 3.21767881768E-05f;
  const T c8 = 2.888167364E-07f;
  const T c9 = 3.960315187E-07f;

  T z;

  bool negate = false;

  // Ensure the conversion to Ting point will give a value in the
  // range (0,0.5] by restricting the input to the bottom half of the
  // input domain. We will later reflect the result if the input was
  // originally in the top half of the input domain
  if (x >= 0x80000000UL) {
    x = 0xffffffffUL - x;
    negate = true;
  }

  // x is now in the range [0,0x80000000) (i.e. [0,0x7fffffff])
  // Convert to Ting point in (0,0.5]
  const T x1 = 1.0f / static_cast<T>(0xffffffffUL);
  const T x2 = x1 / 2.0f;
  T p1 = x * x1 + x2;
  // Convert to Ting point in (-0.5,0]
  T p2 = p1 - 0.5f;

  // The input to the Moro inversion is p2 which is in the range
  // (-0.5,0]. This means that our output will be the negative side
  // of the bell curve (which we will reflect if "negate" is true).

  // Main body of the bell curve for |p| < 0.42
  if (p2 > -0.42f) {
    z = p2 * p2;
    z = p2 * (((a4 * z + a3) * z + a2) * z + a1) /
        ((((b4 * z + b3) * z + b2) * z + b1) * z + 1.0f);
  }
  // Special case (Chebychev) for tail
  else {
    z = log(-log(p1));
    z = -(c1 +
          z * (c2 +
               z * (c3 +
                    z * (c4 +
                         z * (c5 + z * (c6 + z * (c7 + z * (c8 + z * c9))))))));
  }

  // If the original input (x) was in the top half of the range, reflect
  // to get the positive side of the bell curve
  d_Output[pos] = negate ? -z : z;
}

#define WUK_DEFINE_KERNEL_MICND                                                \
  template __global__ void MoroInvCNDgpu<>(float *d_Output, unsigned int pathN)

extern WUK_DEFINE_KERNEL_MICND;

} // namespace MICND

} // namespace wuk