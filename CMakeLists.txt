cmake_minimum_required(VERSION 3.7)

project(GoPTX VERSION 0.0.1)

include(CPack)

add_subdirectory(lib)

add_subdirectory(src)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/GoPTX TYPE INCLUDE)

option(GOPTX_BUILD_TEST "GoPTX_test" OFF)

if(GOPTX_BUILD_TEST)

  include(CTest)

  add_subdirectory(test)

endif()
