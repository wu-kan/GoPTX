#pragma once
#include "../WuKernel.hh"
#include "../device_vector.cuh"
#include "kernel.cuh"
#include <cassert>
namespace wuk {

namespace WMMA {

template <int WMMA_M = 16, int WMMA_N = 16, int WMMA_K = 16, int WARP_SIZE = 32,
          int LAUNCH1D = 1>
struct Wrapper : BaseWrapper {
  wuk::device_vector<__half> va, vb;
  wuk::device_vector<float> vc, vd;
  int _m_ld, _n_ld, _k_ld;
  float _alpha, _beta;
  __half *a, *b;
  float *c, *d;
  Wrapper(const BaseWrapper::LaunchConfig &cfg, int m_ld = 4096,
          int n_ld = 4096, int k_ld = 768, float alpha = 1, float beta = 0)
      : va(m_ld * k_ld), vb(k_ld * n_ld), vc(m_ld * n_ld), vd(m_ld * n_ld),
        a(wuk::raw_pointer_cast(va.data())),
        b(wuk::raw_pointer_cast(vb.data())),
        c(wuk::raw_pointer_cast(vc.data())),
        d(wuk::raw_pointer_cast(vd.data())), _m_ld(m_ld), _n_ld(n_ld),
        _k_ld(k_ld), _alpha(alpha), _beta(beta) {
    if (LAUNCH1D) {
      assert(cfg.blockDim.y == 1);
      assert(cfg.blockDim.z == 1);
      assert(cfg.gridDim.x ==
             ((m_ld + (WMMA_M * cfg.blockDim.x / WARP_SIZE - 1)) /
              (WMMA_M * cfg.blockDim.x / WARP_SIZE)) *
                 ((n_ld + WMMA_N * cfg.blockDim.y - 1) /
                  (WMMA_N * cfg.blockDim.y)));
      assert(cfg.gridDim.y == 1);
      assert(cfg.gridDim.z == 1);
    } else {
      assert(0);
    }
    cudaGetFuncBySymbol(
        &(BaseWrapper::func),
        (const void *)
            simple_wmma_gemm<WMMA_M, WMMA_N, WMMA_K, WARP_SIZE, LAUNCH1D>);
    BaseWrapper::args = std::vector<void *>{
        &a, &b, &c, &d, &_m_ld, &_n_ld, &_k_ld, &_alpha, &_beta};
    BaseWrapper::launchConfig = cfg;
  }
  void reset() {
    wuk::generate<__half>(va, 0, 1, 0);
    wuk::generate<__half>(vb, 0, 1, 1);
    wuk::generate<float>(vc, 0, 1, 2);
    wuk::generate<float>(vc, 0, 1, 3);
  }
  bool equal(const Wrapper &rhs) const {
    if (!wuk::equal<float>(vd, rhs.vd, 1e-3))
      return false;
    return true;
  }
};

extern template struct Wrapper<>;

} // namespace simple_wmma_gemm

} // namespace wuk