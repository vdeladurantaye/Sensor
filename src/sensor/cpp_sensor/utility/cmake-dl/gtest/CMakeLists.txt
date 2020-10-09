cmake_minimum_required(VERSION 2.8.2)
 
project(googletest-download NONE)
 
set(SKIP_GTEST_UPDATE OFF CACHE BOOL "Skip gtest update step when configuring")

include(ExternalProject)
ExternalProject_Add(googletest-dl
  GIT_REPOSITORY    https://github.com/google/googletest.git
  GIT_TAG           master
  SOURCE_DIR        "${CMAKE_SOURCE_DIR}/../../gtest"
  BINARY_DIR        ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  UPDATE_DISCONNECTED ${SKIP_GTEST_UPDATE}
)