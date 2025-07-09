#pragma once
#include <cstdint>

namespace wuk {

namespace STMS {

// https://github.com/hipacc/samples/blob/master/3_Preprocessing/ShiTomasi_Corner/src/main.cpp

template <typename Tin = short, typename Tresult = float,
          typename Tout = uint8_t>
__global__ void goodFeature(int n, Tresult threshold, Tin const *dx,
                            Tin const *dy, Tin const *dxy, Tout *dout) {
  int i = threadIdx.x + blockIdx.x * blockDim.x;
  if (i >= n)
    return;
  const auto x = dx[i];
  const auto y = dy[i];
  const auto xy = dxy[i];
  const auto eigenVIntm =
      sqrt((Tresult)((x - y) * (x - y)) + (Tresult)(4 * xy * xy));
  const auto lambda1 = (Tresult)(0.5) * (x + y + eigenVIntm);
  const auto lambda2 = (Tresult)(0.5) * (x + y - eigenVIntm);
  Tout out = 0;
  if (min(lambda1, lambda2) > threshold)
    out = 1;
  dout[i] = out;
}

#define WUK_DEFINE_KERNEL_STMS                                                 \
  template __global__ void goodFeature<>(int n, float threshold,               \
                                         short const *dx, short const *dy,     \
                                         short const *dxy, uint8_t *dout)

extern WUK_DEFINE_KERNEL_STMS;

}; // namespace STMS

}; // namespace wuk