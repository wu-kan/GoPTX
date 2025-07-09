#pragma once
#include "../WuKernel.hh"
#include "../device_vector.cuh"
#include "kernel.cuh"
#include <cassert>

namespace wuk {

namespace GELU {

template <typename T = __half, typename Tstore = __half2,
          typename T_Op = OP_QuickGelu<float>>
struct Wrapper : BaseWrapper {
  wuk::device_vector<T> vx;
  size_t n;
  T *x;
  Wrapper(const BaseWrapper::LaunchConfig &cfg)
      : vx(size_t(sizeof(Tstore) / sizeof(T)) * cfg.gridDim.x * cfg.blockDim.x),
        x(wuk::raw_pointer_cast(vx.data())), n(vx.size()) {
    assert(cfg.blockDim.y == 1);
    assert(cfg.blockDim.z == 1);
    assert(cfg.gridDim.y == 1);
    assert(cfg.gridDim.z == 1);
    cudaGetFuncBySymbol(&(BaseWrapper::func),
                        (const void *)quickGELU<T, Tstore, T_Op>);
    BaseWrapper::args = std::vector<void *>{&n, &x};
    BaseWrapper::launchConfig = cfg;
  }
  void reset() { wuk::generate<T>(vx, 0, 1); }
  bool equal(const Wrapper &rhs) const {
    if (!wuk::equal<T>(vx, rhs.vx, 1e-3))
      return false;
    return true;
  }
};

extern template struct Wrapper<>;

}; // namespace GELU

}; // namespace wuk