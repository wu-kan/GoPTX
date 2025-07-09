# <https://github.com/antlr/antlr4/blob/4.13.1/runtime/Cpp/cmake/ExternalAntlr4Cpp.cmake>
cmake_minimum_required(VERSION 3.7)

# Bring in the required packages
find_package(antlr4-runtime REQUIRED)

if(NOT EXISTS "${ANTLR4_JAR_LOCATION}")
  file(DOWNLOAD
       https://www.antlr.org/download/antlr-${ANTLR_VERSION}-complete.jar
       ${CMAKE_CURRENT_BINARY_DIR}/antlr-${ANTLR_VERSION}-complete.jar)
  set(ANTLR4_JAR_LOCATION
      ${CMAKE_CURRENT_BINARY_DIR}/antlr-${ANTLR_VERSION}-complete.jar)
endif()

find_package(antlr4-generator REQUIRED)

antlr4_generate(${PROJECT_NAME}_parser ${CMAKE_CURRENT_SOURCE_DIR}/Ptx.g4 BOTH
                TRUE FALSE "${PROJECT_NAME}")

add_library(${PROJECT_NAME}_static STATIC)

install(TARGETS ${PROJECT_NAME}_static)

target_include_directories(${PROJECT_NAME}_static
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_sources(
  ${PROJECT_NAME}_static
  PRIVATE antlr4.cc c.cc cc.cc
          ${ANTLR4_SRC_FILES_${PROJECT_NAME}_parser})

# add directories for generated include files
target_include_directories(
  ${PROJECT_NAME}_static
  PRIVATE ${ANTLR4_INCLUDE_DIR} ${ANTLR4_INCLUDE_DIR_${PROJECT_NAME}_parser})

target_link_libraries(${PROJECT_NAME}_static antlr4_shared)

if(${ANTLR_VERSION} VERSION_LESS_EQUAL 4.10.1)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(UUID REQUIRED uuid)
  target_link_directories(${PROJECT_NAME}_static PUBLIC ${UUID_LIBRARY_DIRS})
  target_link_libraries(${PROJECT_NAME}_static ${UUID_LIBRARIES})
endif()
