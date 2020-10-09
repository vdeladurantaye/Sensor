Sensor
======
This project contains the C++ code for segmenting images like ODLM, using grayscale pixel data. A simple Python interface is provided, it could easily be modified to access more data from the C++ code.

Prerequisites
------------
- OpenCV
- Python
- Cmake: Used for cross-platform project generation
	- Dowload from https://cmake.org/
- Pybind11: Used for creating the python interface
	- Should be downloaded automatically by the Cmake project
	- Otherwise, download from https://github.com/pybind/pybind11 and place in *utility/pybind11*
- Gtest: Used for test management in C++
	- Should be downloaded automatically by the Cmake project
	- Otherwise, download from https://github.com/google/googletest
	- Place in *utility/gtest*


Generate the projects using Cmake
---------------------------------
1. Set the source code folder to the Sensor directory.
2. Set the binaries folder to where you want Cmake to generate the project, typically set as a *./build* folder in the Sensor directory.
3. Click "Configure" and Cmake will attempt to configure the project, however some paths might need to be manually configured
4. Python and OpenCV should be found automatically by cmake, otherwise, manually set the paths.
5. Manually set *NUMPY_INCLUDE_DIR* variable in cmake. 
	- This is typically located in *Python_INSTALL_DIR\Lib\site-packages\numpy\core\include*
6. Pybind11 and Gtest are normally automatically downloaded by the cmake project.
7. If everything succeeds when pressing "Configure", click "Generate" and open the project.


Running the C++ code
--------------------
- Compile the *SENSOR_Test* project.
- Set the working directory of the project to *Sensor/WorkindDir*
- Run the tests, the main is located in *test/test_main.cpp* 
	- The test *Segmentation* might fail if `bool randomInit = true;` because random numbers are not generated the same way on different platforms. The test should work with `bool randomInit = false;`. To regenerate the test validation file with your platform's random numbers, set `bool regenerateValidationFile = true;` and run the test once. Then set it back to `false`

Running from Python
--------------------
- For running the code in Python, the *SENSOR_Python* project has to be compiled in the *RELEASE* configuration. When compiled successfully, the python module is generated in the *bin* folder.
- Run the python file *Sensor.py* in the root directory.

Warning for Mac users
---------------------
This project was developed on Windows. Some features are not fully functional on Mac:
- *Monitor* windows tend to not pop to the foreground when displayed, you may have to look for them each time the program starts.
- *Monitor* windows don't seem be resized properly
- I've been having trouble compiling with XCode.
	- Sometimes, the *NUMPY_INCLUDE_DIR* path doesn't seem to work ('numpy/ndarrayobject.h' not found). As a last resort, I've copied the *numpy* folder from *Python_INSTALL_DIR\Lib\site-packages\numpy\core\include* directly in the *inc* folder of the Sensor directory.
	- The project requires c++11, however Xcode doesn't always detect this. If having trouble compiling, try to change the *C++ Dialect* setting.


