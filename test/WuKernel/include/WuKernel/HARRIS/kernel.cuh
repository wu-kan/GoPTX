#pragma once
#include <cstdint>

namespace wuk {

namespace HARRIS {

// https://github.com/hipacc/samples/blob/master/3_Preprocessing/Harris_Corner/src/main.cpp

template <typename Tin = short, typename Tresult = float,
          typename Tout = uint8_t>
__global__ void harrisCorner(int n, Tresult k, Tresult threshold, Tin const *dx,
                             Tin const *dy, Tin const *dxy, Tout *dout) {
  int i = threadIdx.x + blockIdx.x * blockDim.x;
  if (i >= n)
    return;
  const auto x = dx[i];
  const auto y = dy[i];
  const auto xy = dxy[i];
  Tresult R = ((x * y) - (xy * xy))      /* det   */
              - ((x + y) * (x + y) * k); /* trace */
  Tout out = 0;
  if (R > threshold)
    out = 1;
  dout[i] = out;
}

#define WUK_DEFINE_KERNEL_HARRIS                                               \
  template __global__ void harrisCorner<>(int n, float k, float threshold,     \
                                          short const *dx, short const *dy,    \
                                          short const *dxy, uint8_t *dout)

extern WUK_DEFINE_KERNEL_HARRIS;

}; // namespace HARRIS

}; // namespace wuk