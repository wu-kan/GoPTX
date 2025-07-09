cmake_minimum_required(VERSION 3.18) # CMAKE_CUDA_ARCHITECTURES

project(WuKernel LANGUAGES C CXX CUDA)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third_party/EmbedPTX/)
include(EmbedPTX)

add_library(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB _subs include/WuKernel/*)
foreach(_sub ${_subs})
  if(IS_DIRECTORY ${_sub})
    get_filename_component(_APP ${_sub} NAME_WE ABSOLUTE)

    file(
      WRITE ${CMAKE_CURRENT_BINARY_DIR}/${_APP}/kernel.cu
      "#include \"WuKernel/${_APP}/wrapper.cuh\"\nnamespace wuk{namespace ${_APP}{WUK_DEFINE_KERNEL_${_APP};}}"
    )

    target_sources(${PROJECT_NAME}
                   PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${_APP}/kernel.cu)

    file(
      WRITE ${CMAKE_CURRENT_BINARY_DIR}/${_APP}/wrapper.cu
      "#include \"WuKernel/${_APP}/wrapper.cuh\"\ntemplate struct wuk::${_APP}::Wrapper<>;"
    )
    target_sources(${PROJECT_NAME}
                   PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${_APP}/wrapper.cu)

    foreach(_ARCH ${CMAKE_CUDA_ARCHITECTURES})

      add_library(WuKPTX_${_APP}_${_ARCH} OBJECT
                  ${CMAKE_CURRENT_BINARY_DIR}/${_APP}/kernel.cu)

      set_property(TARGET WuKPTX_${_APP}_${_ARCH} PROPERTY CUDA_ARCHITECTURES
                                                           ${_ARCH})
      set_property(TARGET WuKPTX_${_APP}_${_ARCH} PROPERTY CUDA_PTX_COMPILATION
                                                           ON)
      target_include_directories(WuKPTX_${_APP}_${_ARCH}
                                 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

      file(
        WRITE ${CMAKE_CURRENT_BINARY_DIR}/${_APP}/${_ARCH}/get_ptx.cu
        "#include \"WuKernel/${_APP}/wrapper.cuh\"\n#define kernel wuk_kernel_${_APP}_${_ARCH}\n#include \"get_ptx.c\"\n#undef kernel\nnamespace wuk {template <typename Wrapper, int SM> std::string get_ptx() { return std::string(wuk_kernel_${_APP}_${_ARCH}, wuk_kernel_${_APP}_${_ARCH} + sizeof(wuk_kernel_${_APP}_${_ARCH}) / sizeof(*wuk_kernel_${_APP}_${_ARCH})); } template std::string get_ptx<${_APP}::Wrapper<>, ${_ARCH}>(); }"
      )

      add_library(${PROJECT_NAME}PTX_${_APP}_${_ARCH} OBJECT
                  ${CMAKE_CURRENT_BINARY_DIR}/${_APP}/${_ARCH}/get_ptx.cu)
      target_sources(
        ${PROJECT_NAME}
        PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}PTX_${_APP}_${_ARCH}>)

      embedptx(
        OUTPUT_HEADER_FILE
        ${CMAKE_CURRENT_BINARY_DIR}/${_APP}/${_ARCH}/get_ptx.c
        INPUT_TARGET
        WuKPTX_${_APP}_${_ARCH}
        OUTPUT_TARGETS
        ${PROJECT_NAME}PTX_${_APP}_${_ARCH})

      target_include_directories(
        ${PROJECT_NAME}PTX_${_APP}_${_ARCH}
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${_APP}/${_ARCH})
      target_include_directories(${PROJECT_NAME}PTX_${_APP}_${_ARCH}
                                 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    endforeach()

  endif()
endforeach()

