cmake_minimum_required(VERSION 3.0)

#------------------------------------------------------------------------------
# Download pybind11
#------------------------------------------------------------------------------
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../utility/cmake-dl/pybind11/build")
set(SKIP_PYBIND11_UPDATE ON CACHE BOOL "Skip pybind11 update step when configuring")
# Configure cmake cache
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
	-D SKIP_PYBIND11_UPDATE=${SKIP_PYBIND11_UPDATE} ..
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../utility/cmake-dl/pybind11/build")
	
# Build the download project (performs the actual download)
execute_process(COMMAND ${CMAKE_COMMAND} --build .
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../utility/cmake-dl/pybind11/build")

# Add the pybind11 directory to the project
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../utility/pybind11"
				 "${CMAKE_CURRENT_SOURCE_DIR}/../utility/pybind11/build")
				 
#------------------------------------------------------------------------------
# SENSOR Python
#------------------------------------------------------------------------------
project(SENSOR_Python)

# Get the source files
file(GLOB SENSOR_SOURCES "../src/*.cpp")
# Get the header files
file(GLOB SENSOR_HEADERS "../inc/*.h")

# Get the Python interface files
file(GLOB PY_INTERFACE_FILES "*.cpp" "*.h")
source_group(PythonIterface FILES ${PY_INTERFACE_FILES})

# On Windows, OpenCV is not always found by default, so look in the environment
# variables for OpenCV_DIR
if(DEFINED $ENV{OpenCV_DIR})
	SET(OpenCV_DIR $ENV{OpenCV_DIR})
endif()
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)

# Numpy headers needed for opencv Mat conversion from numpy arrays
get_filename_component(PYTHON_SCRIPT_PATH ${PYTHON_EXECUTABLE} DIRECTORY)
get_filename_component(NUMPY_INCLUDE_TMP 
	"${PYTHON_SCRIPT_PATH}/../Lib/site-packages/numpy/core/include" 
	REALPATH)

set(NUMPY_INCLUDE_DIR "${NUMPY_INCLUDE_TMP}" CACHE PATH "Numpy include directory")
include_directories(${NUMPY_INCLUDE_DIR})

# Define include directories
include_directories("../inc")
include_directories(${Boost_INCLUDE_DIR})

link_directories(${OpenCV_LIB_DIR_OPT})
link_directories(${Boost_LIB_DIR})

# -----------------------------------------------------------------------------
# Create the Python module
# -----------------------------------------------------------------------------
set(OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../sensor")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUT_DIR})

pybind11_add_module(SENSOR_Python ${PY_INTERFACE_FILES} ${SENSOR_SOURCES} 
	${SENSOR_HEADERS})

target_link_libraries(SENSOR_Python PRIVATE ${OpenCV_LIBS})

# Set the name of the python module
set_target_properties(SENSOR_Python PROPERTIES OUTPUT_NAME "cpp_sensor")
# Exclude the debug config from the default build
set_property(TARGET SENSOR_Python
             PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_DEBUG TRUE)

# -----------------------------------------------------------------------------
# Handle configuration-dependent suffix
# -----------------------------------------------------------------------------
# When we have multiple configurations (Debug, Release, etc.), they place their 
# ouput in coresponding directories in the output folder. We want to avoid this
# and have the Release binary directly in the output directory.
set_target_properties(SENSOR_Python PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUT_DIR})

## Alternatively, we could use a generator expression so that the specified folder is used:
## See https://cmake.org/cmake/help/v3.8/prop_tgt/LIBRARY_OUTPUT_DIRECTORY.html
#set_target_properties(library PROPERTIES
#                      LIBRARY_OUTPUT_DIRECTORY my/folder/$<$<CONFIG:Debug>:>)

