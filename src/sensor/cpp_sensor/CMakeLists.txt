cmake_minimum_required(VERSION 3.0)

# Remove Cmake regeneration at build time
SET(CMAKE_SUPPRESS_REGENERATION TRUE)

project(CPP_SENSOR)

add_subdirectory(PythonInterface)
add_subdirectory(test)
