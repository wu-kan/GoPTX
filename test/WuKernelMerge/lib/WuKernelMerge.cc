#include <WuKernelMerge/WuKernelMerge.hh>

#ifndef GOPTX_TEST_NO_USE_NVPTX_COMPILER

#include <cstdio>
#include <nvPTXCompiler.h>

// https://docs.nvidia.com/cuda/ptx-compiler-api/index.html#example-simple-vector-addition

#define NVPTXCOMPILER_SAFE_CALL(x)                                             \
  do {                                                                         \
    nvPTXCompileResult result = x;                                             \
    if (result != NVPTXCOMPILE_SUCCESS) {                                      \
      std::fprintf(stderr, "\n---\n%s\n---\n", ptx.c_str());                   \
      std::fprintf(stderr,                                                     \
                   "{\"File\": \"%s\", \"Line\": %d, \"Error\": "              \
                   "\"%s\",\"Code\": %d}\n",                                   \
                   __FILE__, __LINE__, #x, (int)result);                       \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)

int wuk::get_elf(const std::string &ptx, std::vector<int8_t> &elf, int sm,
                 int reglimit, const std::string &kernel_name) {
  char arch[31], reg[31] = "-maxrregcount=32";
  std::sprintf(arch, "-arch=sm_%d", sm);
  std::sprintf(reg, "-maxrregcount=%d", reglimit);
  const char *compile_options[] = {"-O 3", "-v", arch, reg};
  int size_options = sizeof(compile_options) / sizeof(compile_options[0]);
  if (reglimit < 0)
    --size_options;
  nvPTXCompilerHandle compiler = NULL;
  NVPTXCOMPILER_SAFE_CALL(
      nvPTXCompilerCreate(&compiler, ptx.size(), ptx.c_str()));
  NVPTXCOMPILER_SAFE_CALL(
      nvPTXCompilerCompile(compiler, size_options, compile_options));
  size_t elfSize = 0, infoSize = 0, regUse = 0;

  NVPTXCOMPILER_SAFE_CALL(
      nvPTXCompilerGetCompiledProgramSize(compiler, &elfSize));
  elf.assign(elfSize, 0);
  NVPTXCOMPILER_SAFE_CALL(
      (nvPTXCompilerGetCompiledProgram(compiler, (void *)elf.data())));

  if (true || !kernel_name.empty()) {
    NVPTXCOMPILER_SAFE_CALL(nvPTXCompilerGetInfoLogSize(compiler, &infoSize));
    if (infoSize != 0) {
      std::vector<char> infoLog(infoSize + 1, 0);
      NVPTXCOMPILER_SAFE_CALL(
          nvPTXCompilerGetInfoLog(compiler, infoLog.data()));
      while (!infoLog.empty() && infoLog.back() == 0)
        infoLog.pop_back();
      std::string info(infoLog.begin(), infoLog.end());
#if 0
$ ptxas -arch=sm_80 -v a.ptx
ptxas info    : 0 bytes gmem
ptxas info    : Compiling entry function 'merged_kernel' for 'sm_80'
ptxas info    : Function properties for merged_kernel
    0 bytes stack frame, 0 bytes spill stores, 0 bytes spill loads
ptxas info    : Used 30 registers, 380 bytes cmem[0]
#endif
#if 0
      std::fprintf(stderr, "Info log: %s\n", info.c_str());
#endif
      if (!kernel_name.empty()) {
        info = info.substr(info.find(kernel_name));
      }
      info = info.substr(0, info.find(" registers, "));
      info = info.substr(info.find("Used "));
      info = info.substr(info.find(" "));
#if 0
      std::fprintf(stderr, "regUse: %s\n", info.c_str());
      std::exit(0);
#endif
      regUse = std::stoi(info);
    }
  }

  NVPTXCOMPILER_SAFE_CALL(nvPTXCompilerDestroy(&compiler));

  return regUse;
}

#else

int wuk::get_elf(const std::string &ptx, std::vector<int8_t> &elf, int sm,
                 int reglimit) {
  elf.assign(ptx.begin(), ptx.end());
  elf.push_back(0);
  return 0;
}

#endif

std::string wuk::get_ptx_merged(const std::string &ptx_first,
                                const std::string &ptx_second,
                                const std::string &name_first,
                                const std::string &name_second,
                                const GoPTX_config config,
                                const std::string &name_merged) {
  std::string ptx_merged;
  do {
    const std::string suffix_first = "w", suffix_second = "k";
    char *ptx_concated_c_str;
    GoPTX_link(&ptx_concated_c_str, ptx_first.c_str(), ptx_second.c_str(),
               suffix_first.c_str(), suffix_second.c_str());
    char *ptx_merged_c_str;
    GoPTX_merge(&ptx_merged_c_str, ptx_concated_c_str, name_merged.c_str(),
                (name_first + ".*" + suffix_first + "$").c_str(),
                (name_second + ".*" + suffix_second + "$").c_str(), config);
    ptx_merged.assign(ptx_merged_c_str);
    GoPTX_free(ptx_concated_c_str);
    GoPTX_free(ptx_merged_c_str);
  } while (0);
  return ptx_merged;
}

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

wuk::CompiledWrapper::~CompiledWrapper() {
  DRIVER_API_CALL(cuModuleUnload(module));
}

wuk::CompiledWrapper::CompiledWrapper(const wuk::BaseWrapper::LaunchConfig &cfg,
                                      const std::vector<void *> &args,
                                      const std::string &ptx, int sm, int reg,
                                      const std::string &name_merged) {
  wuk::BaseWrapper::launchConfig = cfg;
  wuk::BaseWrapper::args = args;
  std::vector<int8_t> elf;
  int real_reg = wuk::get_elf(ptx, elf, sm, reg, name_merged);
  // std::fprintf(stderr, "(%s) reg usage: %d\n", name_merged.c_str(),
  // real_reg);
  DRIVER_API_CALL(cuModuleLoadData(&module, elf.data()));
  DRIVER_API_CALL(cuModuleGetFunction(&(wuk::BaseWrapper::func), module,
                                      name_merged.c_str()));
}

std::vector<void *> concat(const std::vector<void *> &args0,
                           const std::vector<void *> &args1) {
  auto ret = args0;
  ret.insert(ret.end(), args1.begin(), args1.end());
  return ret;
}

int get_merged_reg(const std::string &ptx_first, const std::string &ptx_second,
                   const std::string &name_first,
                   const std::string &name_second, int sm) {
  std::vector<int8_t> elf;
  std::vector<int> arr{32, 40, 48, 56, 64, 72, 80, 96, 128, 168, 256};
  int reg_first = wuk::get_elf(ptx_first, elf, sm, -1, name_first),
      reg_second = wuk::get_elf(ptx_second, elf, sm, -1, name_second);
  int reg = std::max(reg_first, reg_second);
  for (auto it : arr) {
    if (reg <= it) {
      reg = it;
      break;
    }
  }
  return reg;
}

wuk::MergedWrapper::MergedWrapper(
    const wuk::BaseWrapper::LaunchConfig &cfg, wuk::BaseWrapper &_first,
    wuk::BaseWrapper &_second, const std::string &name_first,
    const std::string &name_second, const std::string &ptx_first,
    const std::string &ptx_second, const int sm, const GoPTX_config config,
    const std::string &name_merged)
    : CompiledWrapper(
          cfg,
          concat(_first.wuk::BaseWrapper::args, _second.wuk::BaseWrapper::args),
          wuk::get_ptx_merged(ptx_first, ptx_second, name_first, name_second,
                              config, name_merged),
          sm,
          get_merged_reg(ptx_first, ptx_second, name_first, name_second, sm),
          name_merged) {}
