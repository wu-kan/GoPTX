#pragma once
#include "../WuKernel.hh"
#include "../device_vector.cuh"
#include "kernel.cuh"
#include <cassert>

namespace wuk {

namespace MICND {

template <typename T = float> struct Wrapper : BaseWrapper {
  wuk::device_vector<T> vd_Output;
  T *d_Output;
  unsigned int pathN;
  Wrapper(const BaseWrapper::LaunchConfig &cfg)
      : vd_Output(cfg.gridDim.x * cfg.blockDim.x),
        d_Output(wuk::raw_pointer_cast(vd_Output.data())),
        pathN(cfg.gridDim.x * cfg.blockDim.x) {
    assert(cfg.blockDim.y == 1);
    assert(cfg.blockDim.z == 1);
    assert(cfg.gridDim.y == 1);
    assert(cfg.gridDim.z == 1);
    cudaGetFuncBySymbol(&(BaseWrapper::func), (const void *)MoroInvCNDgpu<T>);
    BaseWrapper::args = std::vector<void *>{&d_Output, &pathN};
    BaseWrapper::launchConfig = cfg;
  }
  void reset() {}
  bool equal(const Wrapper &rhs) const {
    if (!wuk::equal<T>(vd_Output, rhs.vd_Output, 1e-3))
      return false;
    return true;
  }
};

extern template struct Wrapper<>;

}; // namespace moroInvCND

}; // namespace wuk