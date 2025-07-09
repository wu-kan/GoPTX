#pragma once

#include <GoPTX/GoPTX.h>
#include <WuKernel/WuKernel.hh>
#include <WuKernelMerge/WuKernelMerge.hh>
#include <cstdio>
#include <cuda.h>
#include <functional>
#include <string>

#define DRIVER_API_CALL(x)                                                     \
  do {                                                                         \
    CUresult result = x;                                                       \
    if (result != CUDA_SUCCESS) {                                              \
      const char *msg;                                                         \
      cuGetErrorName(result, &msg);                                            \
      std::fprintf(stderr,                                                     \
                   "{\"File\": "                                               \
                   "\"%s\", \"Line\": %d, \"Error\": \"%s\",\"Code\": %d, "    \
                   "\"Msg\": \"%s\"}\n",                                       \
                   __FILE__, __LINE__, #x, (int)result, msg);                  \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)

#define LAUNCH(w, s)                                                           \
  cuLaunchKernel(w.func, w.launchConfig.gridDim.x, w.launchConfig.gridDim.y,   \
                 w.launchConfig.gridDim.z, w.launchConfig.blockDim.x,          \
                 w.launchConfig.blockDim.y, w.launchConfig.blockDim.z,         \
                 w.launchConfig.dynamicSmemBytes, s, w.args.data(), nullptr)

namespace GoPTX {

namespace test {

extern int now_cnt;

std::string test();

std::string metric(const std::string &prepend,
                   const std::function<void()> &reset,
                   const std::function<void()> &kernel, int test_time = 9);

#if 0 // to run slower for safe
#define GOPTX_TEST_CHECK
#endif

template <typename Wrapper_first, typename Wrapper_second, int... SMs>
std::string workload(const wuk::BaseWrapper::LaunchConfig &cfg,
                     const std::string &name_first,
                     const std::string &name_second) {
  CUdevice device;
  DRIVER_API_CALL(cuDeviceGet(&device, 0));
  int major, minor;
  DRIVER_API_CALL(cuDeviceGetAttribute(
      &major, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, device));
  DRIVER_API_CALL(cuDeviceGetAttribute(
      &minor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, device));

  const std::string append =
      "\"First\": \"" + name_first + "\", \"Second\": \"" + name_second +
      "\", \"Workload\": \"" + name_first + "+" + name_second + "\"";
  std::fprintf(stderr, "[%d]: {%s}\n", now_cnt++, append.c_str());

  std::string ret;
  Wrapper_first _first(cfg);
  Wrapper_second _second(cfg);
#ifndef MEGREPTX_TEST_NO_RESET
  auto reset = [&] {
    _first.reset();
    _second.reset();
  };
#else
  auto reset = [&] {};
#endif
  const int nStreams = 2;
  CUstream s[nStreams];
  for (int i = 0; i < nStreams; ++i) {
    DRIVER_API_CALL(cuStreamCreate(&s[i], CU_STREAM_NON_BLOCKING));
    DRIVER_API_CALL(cuStreamSynchronize(s[i]));
  }
  do {
#ifdef GOPTX_TEST_CHECK
    Wrapper_first ref_first(cfg);
    Wrapper_second ref_second(cfg);
    ref_first.reset();
    ref_second.reset();
    DRIVER_API_CALL(LAUNCH(ref_first, s[0]));
    DRIVER_API_CALL(LAUNCH(ref_second, s[0]));
    DRIVER_API_CALL(cuStreamSynchronize(s[0]));
    auto check = [&] {
      if (!_first.equal(ref_first) || !_second.equal(ref_second)) {
        std::fprintf(stderr, "{\"First\": \"%s\", \"Second\": \"%s\"}\n",
                     name_first.c_str(), name_second.c_str());
        std::exit(-1);
      }
    };
#else
    auto check = [&] {};
#endif

    const auto ptx_first =
                   wuk::get_ptx<Wrapper_first, SMs...>(major * 10 + minor),
               ptx_second =
                   wuk::get_ptx<Wrapper_second, SMs...>(major * 10 + minor);
    ret += metric(append + ", \"Strategy\": \"cuStream\"", reset, [&] {
      DRIVER_API_CALL(LAUNCH(_first, s[0]));
      DRIVER_API_CALL(LAUNCH(_second, s[1]));
      DRIVER_API_CALL(cuStreamSynchronize(s[0]));
      DRIVER_API_CALL(cuStreamSynchronize(s[1]));
    });
    ret += ",";
#ifdef TESTGOPTX_STRATEGY_FOR
#error
#else
#define TESTGOPTX_STRATEGY_FOR(STRATEGY)                                       \
  do {                                                                         \
    GoPTX_config config;                                                       \
    config.strategy = STRATEGY;                                                \
    config.no_avoid_deadlock = 0;                                              \
    config.no_avoid_aligned = 1;                                               \
    auto cfg1 = cfg;                                                           \
    if (config.strategy == GoPTX_strategy_hfuse) {                             \
      cfg1.blockDim.z *= 2;                                                    \
    }                                                                          \
    wuk::MergedWrapper merged(                                                 \
        cfg1, _first, _second, name_first, name_second, ptx_first, ptx_second, \
        major * 10 + minor, config,                                            \
        "merged__" + name_first + "__" + name_second + "__" + #STRATEGY);      \
    auto stg = std::string(#STRATEGY);                                         \
    auto json =                                                                \
        metric(append + ", \"Strategy\": \"" + stg + "\"", reset, [&] {        \
          DRIVER_API_CALL(LAUNCH(merged, s[0]));                               \
          DRIVER_API_CALL(cuStreamSynchronize(s[0]));                          \
        });                                                                    \
    check();                                                                   \
    ret += json + ",";                                                         \
  } while (0)
    TESTGOPTX_STRATEGY_FOR(GoPTX_strategy_vfuse);
    TESTGOPTX_STRATEGY_FOR(GoPTX_strategy_hfuse);
    TESTGOPTX_STRATEGY_FOR(GoPTX_strategy_GoPTX_latency);
#undef TESTGOPTX_STRATEGY_FOR
#endif
#ifdef GOPTX_TEST_FULL // You can skip these to run faster

    ret +=
        metric(append + ", \"Strategy\": \"serial first second\"", reset, [&] {
          DRIVER_API_CALL(LAUNCH(_first, s[0]));
          DRIVER_API_CALL(LAUNCH(_second, s[0]));
          DRIVER_API_CALL(cuStreamSynchronize(s[0]));
        });
    ret += ",";

    ret +=
        metric(append + ", \"Strategy\": \"serial second first\"", reset, [&] {
          DRIVER_API_CALL(LAUNCH(_second, s[1]));
          DRIVER_API_CALL(LAUNCH(_first, s[1]));
          DRIVER_API_CALL(cuStreamSynchronize(s[1]));
        });
    ret += ",";
#endif
    ret += metric(append + ", \"Strategy\": \"only first\"", reset, [&] {
      DRIVER_API_CALL(LAUNCH(_first, s[0]));
      DRIVER_API_CALL(cuStreamSynchronize(s[0]));
    });
    ret += ",";
    ret += metric(append + ", \"Strategy\": \"only second\"", reset, [&] {
      DRIVER_API_CALL(LAUNCH(_second, s[1]));
      DRIVER_API_CALL(cuStreamSynchronize(s[1]));
    });
    ret += ",";

  } while (0);

  if (ret.back() == ',')
    ret.pop_back();

  for (int i = 0; i < nStreams; ++i) {
    DRIVER_API_CALL(cuStreamDestroy(s[i]));
  }
  return ret;
}

} // namespace test

} // namespace GoPTX
