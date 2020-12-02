cmake_minimum_required(VERSION 3.0)

# Remove Cmake regeneration at build time
SET(CMAKE_SUPPRESS_REGENERATION TRUE)

#------------------------------------------------------------------------------
# Download gtest
#------------------------------------------------------------------------------
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../utility/cmake-dl/gtest/build")
set(SKIP_GTEST_UPDATE ON CACHE BOOL "Skip gtest update step when configuring")
# Configure cmake cache
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
	-D SKIP_GTEST_UPDATE=${SKIP_GTEST_UPDATE} ..
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../utility/cmake-dl/gtest/build")
	
# Build the download project (performs the actual download)
execute_process(COMMAND ${CMAKE_COMMAND} --build .
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../utility/cmake-dl/gtest/build")

	
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
 
# Configure GoogleTest cmake options to only build gtest (not using gmock)
option(BUILD_GTEST "" ON)
option(BUILD_GMOCK "" OFF)
# Also since we embed gtest, not need to install it
option(INSTALL_GTEST "" OFF)

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../utility/gtest"
                 "${CMAKE_CURRENT_SOURCE_DIR}/../utility/gtest/build_${CMAKE_GENERATOR}")
				 

#------------------------------------------------------------------------------
# SENSOR_Tests
#------------------------------------------------------------------------------
project(SENSOR_Tests)

# Get the source files
file(GLOB SENSOR_SOURCES "../src/*.cpp")
# Get the header files
file(GLOB SENSOR_HEADERS "../inc/*.h")

# Get the test files
file(GLOB SENSOR_TESTS *.cpp *.h)
source_group(Tests "test_*")

# On Windows, OpenCV is not always found by default, so look in the environment
# variables for OpenCV_DIR
if(DEFINED $ENV{OpenCV_DIR})
	SET(OpenCV_DIR $ENV{OpenCV_DIR})
endif()
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)

# Define include directories
include_directories("../inc")
include_directories(${Boost_INCLUDE_DIR})

include_directories(${GTEST_INCLUDE_DIR})

link_directories(${Boost_LIB_DIR})
link_directories(${OpenCV_LIB_DIR_OPT})

# ----------------------------------------------------------------------------
# Create the Test project
# ----------------------------------------------------------------------------
add_executable(SENSOR_Tests ${SENSOR_TESTS} ${SENSOR_SOURCES} ${SENSOR_HEADERS})

target_link_libraries(SENSOR_Tests ${OpenCV_LIBS})
target_link_libraries(SENSOR_Tests gtest)


set_target_properties(
    SENSOR_Tests PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/WorkingDir")

