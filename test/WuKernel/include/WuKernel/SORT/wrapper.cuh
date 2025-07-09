#pragma once
#include "../WuKernel.hh"
#include "../device_vector.cuh"
#include "kernel.cuh"
#include <cassert>

namespace wuk {

namespace SORT {

template <int LOG_DIVISIONS = 5> struct Wrapper : BaseWrapper {
  wuk::device_vector<float> vinput, voutput;
  wuk::device_vector<unsigned int> vindice, vd_prefixoffsets, vl_offsets;
  float *input;
  unsigned int *indice;
  float *output;
  int size;
  unsigned int *d_prefixoffsets;
  unsigned int *l_offsets;
  Wrapper(const BaseWrapper::LaunchConfig &cfg)
      : vinput(cfg.gridDim.x * cfg.blockDim.x),
        input(wuk::raw_pointer_cast(vinput.data())),
        vindice(cfg.gridDim.x * cfg.blockDim.x),
        indice(wuk::raw_pointer_cast(vindice.data())),
        voutput(cfg.gridDim.x * cfg.blockDim.x),
        output(wuk::raw_pointer_cast(voutput.data())),
        vd_prefixoffsets(cfg.gridDim.x * cfg.blockDim.x / 32 *
                         size_t(1 << LOG_DIVISIONS)),
        d_prefixoffsets(wuk::raw_pointer_cast(vd_prefixoffsets.data())),
        vl_offsets(1 << LOG_DIVISIONS),
        l_offsets(wuk::raw_pointer_cast(vl_offsets.data())),
        size(cfg.gridDim.x * cfg.blockDim.x) {
    assert(cfg.blockDim.y == 1);
    assert(cfg.blockDim.z == 1);
    assert(cfg.gridDim.y == 1);
    assert(cfg.gridDim.z == 1);
    cudaGetFuncBySymbol(&(BaseWrapper::func),
                        (const void *)bucketsort<LOG_DIVISIONS>);
    BaseWrapper::args = std::vector<void *>{
        &input, &indice, &output, &size, &d_prefixoffsets, &l_offsets};
    BaseWrapper::launchConfig = cfg;
  }
  void reset() {
    wuk::generate<float>(vinput, 0, 1, 0);
    wuk::generate<unsigned int>(vindice, 0, vinput.size(), 1);
    wuk::generate<unsigned int>(vd_prefixoffsets, 0, 1 << LOG_DIVISIONS, 2);
    wuk::generate<unsigned int>(vl_offsets, 0, 1, 3);
    wuk::generate<float>(voutput, 0, 1, 4);
  }
  bool equal(const Wrapper &rhs) const {
    if (!wuk::equal<float>(voutput, rhs.voutput, 1e-3))
      return false;
    return true;
  }
};

extern template struct Wrapper<>;

}; // namespace SORT

}; // namespace wuk