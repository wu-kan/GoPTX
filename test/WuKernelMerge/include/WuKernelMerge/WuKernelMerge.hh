#pragma once
#include <GoPTX/GoPTX.h>
#include <WuKernel/WuKernel.hh>
#include <cuda.h>
#include <string>
#include <vector>

namespace wuk {

int get_elf(const std::string &ptx, std::vector<int8_t> &elf, int sm,
            int reglimit = 0,
            const std::string &kernel_name = ""); // return real reg usage

std::string
get_ptx_merged(const std::string &ptx_first, const std::string &ptx_second,
               const std::string &name_first, const std::string &name_second,
               const GoPTX_config config, const std::string &name_merged);

struct CompiledWrapper : wuk::BaseWrapper {
  CUmodule module;
  ~CompiledWrapper();
  CompiledWrapper(const wuk::BaseWrapper::LaunchConfig &cfg,
                  const std::vector<void *> &args, const std::string &ptx,
                  int sm, int reg, const std::string &name_merged);
};

struct MergedWrapper : wuk::CompiledWrapper {
  MergedWrapper(const wuk::BaseWrapper::LaunchConfig &cfg,
                wuk::BaseWrapper &_first, wuk::BaseWrapper &_second,
                const std::string &name_first, const std::string &name_second,
                const std::string &ptx_first, const std::string &ptx_second,
                const int sm, const GoPTX_config config,
                const std::string &name_merged);
};

} // namespace wuk
