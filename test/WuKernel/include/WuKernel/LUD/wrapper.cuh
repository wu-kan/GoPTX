#pragma once
#include "../WuKernel.hh"
#include "../device_vector.cuh"
#include "kernel.cuh"
#include <cassert>
#include <cmath>

namespace wuk {

namespace LUD {

template <typename ElTp = float, int BLOCK_SIZE = 16, int LAUNCH1D = 1>
struct Wrapper : BaseWrapper {
  int matrix_dim, i;
  wuk::device_vector<ElTp> vm;
  ElTp *m;
  Wrapper(const BaseWrapper::LaunchConfig &cfg)
      : matrix_dim(std::sqrt(cfg.blockDim.x * cfg.gridDim.x)),
        vm(1LL * matrix_dim * matrix_dim), i(0),
        m(wuk::raw_pointer_cast(vm.data())) {
    if (LAUNCH1D) {
      assert(cfg.blockDim.x == BLOCK_SIZE * BLOCK_SIZE);
      assert(cfg.blockDim.y == 1);
      assert(cfg.blockDim.z == 1);
      assert(cfg.gridDim.y == 1);
      assert(cfg.gridDim.z == 1);
      assert(vm.size() == cfg.blockDim.x * cfg.gridDim.x);
    } else {
      assert(0);
    }
    cudaGetFuncBySymbol(&(BaseWrapper::func),
                        (const void *)lud_internal<ElTp, BLOCK_SIZE, LAUNCH1D>);
    BaseWrapper::args = std::vector<void *>{&m, &matrix_dim, &i};
    BaseWrapper::launchConfig = cfg;
  }
  void reset() { wuk::generate<ElTp>(vm, 0, 1); }
  bool equal(const Wrapper &rhs) const {
    if (!wuk::equal<ElTp>(vm, rhs.vm, 1e-3))
      return false;
    return true;
  }
};

extern template struct Wrapper<>;

}; // namespace LUD

}; // namespace wuk