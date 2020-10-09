cmake_minimum_required(VERSION 2.8.2)
 
project(pybind11-download NONE)

set(SKIP_PYBIND11_UPDATE OFF CACHE BOOL "Skip pybind11 update step when configuring")
  
include(ExternalProject)
ExternalProject_Add(pybind11-dl
  GIT_REPOSITORY    https://github.com/pybind/pybind11.git
  GIT_TAG           master
  SOURCE_DIR        "${CMAKE_SOURCE_DIR}/../../pybind11"
  BINARY_DIR        ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  UPDATE_DISCONNECTED ${SKIP_PYBIND11_UPDATE}
)