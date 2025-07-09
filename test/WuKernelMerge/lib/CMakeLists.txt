cmake_minimum_required(VERSION 3.25) # CUDA::nvptxcompiler_static

project(WuKernelMerge LANGUAGES CXX)

add_library(${PROJECT_NAME} ${PROJECT_NAME}.cc)

install(TARGETS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_include_directories(
  ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../WuKernel/include)

target_include_directories(
  ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../include)
target_link_libraries(${PROJECT_NAME} GoPTX_static)

find_package(CUDAToolkit REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE ${CUDAToolkit_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} CUDA::cuda_driver CUDA::nvptxcompiler_static)
