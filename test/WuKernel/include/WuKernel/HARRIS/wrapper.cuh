#pragma once
#include "../WuKernel.hh"
#include "../device_vector.cuh"
#include "kernel.cuh"
#include <cassert>

namespace wuk {

namespace HARRIS {

template <typename Tin = short, typename Tresult = float,
          typename Tout = uint8_t>
struct Wrapper : BaseWrapper {
  wuk::device_vector<Tin> vdx, vdy, vdxy;
  wuk::device_vector<Tout> vdout;
  int n;
  Tresult k;
  Tresult threshold;
  Tin const *dx;
  Tin const *dy;
  Tin const *dxy;
  Tout *dout;
  Wrapper(const BaseWrapper::LaunchConfig &cfg)
      : vdx(cfg.gridDim.x * cfg.blockDim.x),
        dx(wuk::raw_pointer_cast(vdx.data())),
        vdy(cfg.gridDim.x * cfg.blockDim.x),
        dy(wuk::raw_pointer_cast(vdy.data())),
        vdxy(cfg.gridDim.x * cfg.blockDim.x),
        dxy(wuk::raw_pointer_cast(vdxy.data())),
        vdout(cfg.gridDim.x * cfg.blockDim.x),
        dout(wuk::raw_pointer_cast(vdout.data())),
        n(cfg.gridDim.x * cfg.blockDim.x), k(0.04f), threshold(20000.0f) {
    assert(cfg.blockDim.y == 1);
    assert(cfg.blockDim.z == 1);
    assert(cfg.gridDim.y == 1);
    assert(cfg.gridDim.z == 1);
    cudaGetFuncBySymbol(&(BaseWrapper::func),
                        (const void *)harrisCorner<Tin, Tresult, Tout>);
    BaseWrapper::args =
        std::vector<void *>{&n, &k, &threshold, &dx, &dy, &dxy, &dout};
    BaseWrapper::launchConfig = cfg;
  }
  void reset() {
    wuk::generate<Tin>(vdx, 0, 180);
    wuk::generate<Tin>(vdy, 0, 180);
    wuk::generate<Tin>(vdxy, 0, 180);
  }
  bool equal(const Wrapper &rhs) const {
    if (!wuk::equal<Tout>(vdout, rhs.vdout, 1))
      return false;
    return true;
  }
};

extern template struct Wrapper<>;

}; // namespace HARRIS

}; // namespace wuk