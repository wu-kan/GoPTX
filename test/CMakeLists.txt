cmake_minimum_required(VERSION 3.17)

project(GoPTX_test LANGUAGES CUDA)

add_executable(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME})

find_package(CUDAToolkit REQUIRED)

target_link_libraries(${PROJECT_NAME} CUDA::cudart_static CUDA::cuda_driver)

add_subdirectory(third_party/wuk_cupti_wrapper/lib)

target_link_libraries(${PROJECT_NAME} wuk_cupti_wrapper)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/wuk_cupti_wrapper/include)

add_subdirectory(WuKernel)

add_subdirectory(WuKernelMerge)

target_include_directories(${PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(${PROJECT_NAME} WuKernel)

target_include_directories(${PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/WuKernel/include)

target_link_libraries(${PROJECT_NAME} WuKernelMerge)

target_include_directories(
  ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/WuKernelMerge/include)

target_sources(${PROJECT_NAME} PRIVATE main.cc metric.cc)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set(ALL_IN_ONE_HEADER "#include \"test.hh\"\n")

file(GLOB _subs ${CMAKE_CURRENT_SOURCE_DIR}/WuKernel/include/WuKernel/*)

foreach(_sub IN LISTS _subs)
  if(IS_DIRECTORY ${_sub})
    get_filename_component(_APP ${_sub} NAME_WE ABSOLUTE)
    string(APPEND ALL_IN_ONE_HEADER "#include <WuKernel/${_APP}/wrapper.cuh>\n")
  endif()
endforeach()

list(JOIN CMAKE_CUDA_ARCHITECTURES "," WUK_CUDA_ARCHITECTURES)

set(TESTER_SOUCE "#include \"workload_all_in_one.cuh\"\n")

string(APPEND TESTER_SOUCE "namespace GoPTX::test {\n")

string(APPEND TESTER_SOUCE "std::string test() {\n")

string(APPEND TESTER_SOUCE "std::string json;")

set(APP0_LIST)
set(APP1_LIST)
set(BLOCKDIM_LIST)
set(GRIDDIM_LIST)

macro(ADD_workload _APP0 _APP1 BLOCKDIM GRIDDIM)
  list(APPEND APP0_LIST ${_APP0})
  list(APPEND APP1_LIST ${_APP1})
  list(APPEND BLOCKDIM_LIST ${BLOCKDIM})
  list(APPEND GRIDDIM_LIST ${GRIDDIM})
endmacro()


set(APP_LIST GELU;HARRIS;LUD;MICND;SORT;STMS;WMMA)

foreach(_APP0 IN LISTS APP_LIST)

  foreach(_APP1 IN LISTS APP_LIST)

    ADD_workload(${_APP0} ${_APP1} 256 1048576)

  endforeach()

endforeach()

foreach(
  _APP0
  _APP1
  BLOCKDIM
  GRIDDIM
  IN
  ZIP_LISTS
  APP0_LIST
  APP1_LIST
  BLOCKDIM_LIST
  GRIDDIM_LIST)

  string(
    APPEND
    ALL_IN_ONE_HEADER
    "extern template std::string GoPTX::test::workload<wuk::${_APP0}::Wrapper<>,wuk::${_APP1}::Wrapper<>>(const wuk::BaseWrapper::LaunchConfig &cfg, const std::string &name_first, const std::string &name_second);\n"
  )

  file(
    WRITE ${CMAKE_CURRENT_BINARY_DIR}/workload_${_APP0}_${_APP1}.cu
    "#include \"test.hh\"\n #include <WuKernel/${_APP0}/wrapper.cuh>\n #include <WuKernel/${_APP1}/wrapper.cuh>\n template std::string GoPTX::test::workload<wuk::${_APP0}::Wrapper<>,wuk::${_APP1}::Wrapper<>>(const wuk::BaseWrapper::LaunchConfig &cfg, const std::string &name_first, const std::string &name_second);\n"
  )

  target_sources(
    ${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/workload_${_APP0}_${_APP1}.cu)

  string(APPEND TESTER_SOUCE "do{wuk::BaseWrapper::LaunchConfig cfg;")

  string(
    APPEND
    TESTER_SOUCE
    "cfg.blockDim = ${BLOCKDIM}; cfg.gridDim = ${GRIDDIM}; cfg.dynamicSmemBytes = 0;"
  )
  string(
    APPEND
    TESTER_SOUCE
    "json += GoPTX::test::workload<wuk::${_APP0}::Wrapper<>,wuk::${_APP1}::Wrapper<>,${WUK_CUDA_ARCHITECTURES}>(cfg, \"${_APP0}\", \"${_APP1}\") + \",\"; } while(0);\n"
  )
endforeach()

string(APPEND TESTER_SOUCE "if (json.back() == \',\')json.pop_back();")
string(APPEND TESTER_SOUCE "json = \"[\" + json + \"]\";")
string(APPEND TESTER_SOUCE "return json;}}")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test.cu "${TESTER_SOUCE}")

target_sources(${PROJECT_NAME}
               PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/test.cu)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/workload_all_in_one.cuh
     "${ALL_IN_ONE_HEADER}")

add_test(
  NAME ${PROJECT_NAME}
  COMMAND
    sh -c
    "$<TARGET_FILE:${PROJECT_NAME}> > ${CMAKE_CURRENT_BINARY_DIR}/results.html")
