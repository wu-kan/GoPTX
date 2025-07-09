#pragma once
#include <cuda_runtime.h>
#include <string>
#include <vector>

namespace wuk {

struct BaseWrapper {
#if 1
  typedef cudaLaunchConfig_t LaunchConfig;
#else
  typedef struct LaunchConfig {
    dim3 blockDim;
    dim3 gridDim;
    size_t dynamicSmemBytes;
  } LaunchConfig;
#endif
  LaunchConfig launchConfig;
  cudaFunction_t func;
  std::vector<void *> args;
};

template <typename Wrapper, int SM> std::string get_ptx();

template <typename Wrapper> std::string get_ptx(int sm) {
  return "// WuKernel did not build for sm_" + std::to_string(sm) + "\n";
}

template <typename Wrapper, int SM, int... SMs> std::string get_ptx(int sm) {
  return sm == SM ? get_ptx<Wrapper, SM>() : get_ptx<Wrapper, SMs...>(sm);
}
}; // namespace wuk