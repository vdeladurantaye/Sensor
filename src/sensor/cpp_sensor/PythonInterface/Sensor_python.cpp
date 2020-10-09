
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include <iostream>
using namespace std;


#include "Sensor_python.h"
#include "PixelLayer.h"
#include "LayerDebugger.h"
#include "Monitor.h"

// ndarray_converter.h is needed to import/export cv::Mat to numpy's ndarray
#include "ndarray_converter.h"
#include "PyDictConversion.h"
#include <boost/property_tree/ini_parser.hpp>


//=============================================================================
//							   Static functions
//=============================================================================
void SetWorkingDir(const string& a_working_dir)
{
	Config::WORKING_DIR = a_working_dir;
}
//-----------------------------------------------------------------------------
void AddDebugger(const string& a_name,
				 const SegmentationLayer& a_layer)
{
	LayerDebugger::AddDebugger(a_layer, a_name);
}
//-----------------------------------------------------------------------------
void LoadConfigFile(const string& a_filename)
{
	Config::LoadConfigFile(a_filename);
}

//-----------------------------------------------------------------------------
void SetConfig(pybind11::dict a_dict)
{
	boost::property_tree::ptree tree = ConvertDictToPtree(a_dict);
	Config::SetConfig(tree);
	//try { boost::property_tree::ini_parser::write_ini("test.ini", tree); }
	//catch (...) {}
}

//=============================================================================
//								Pybind11 Module
//=============================================================================
PYBIND11_MODULE(cpp_sensor, m)
{
	NDArrayConverter::init_numpy();

	m.def("SetWorkingDir", &SetWorkingDir);
	m.def("AddDebugger", &AddDebugger);
	m.def("LoadConfigFile", &LoadConfigFile);
	m.def("SetConfig", &SetConfig);

	py::class_<SegmentationLayer, PySegLayer>(m, "SegLayer")
		.def(py::init<const cv::Mat&>())
		.def("SegmentLayer", &SegmentationLayer::SegmentLayer)
		.def("GetCoefStabilization", 
			 &SegmentationLayer::GetCoefStabilization,
			 py::arg("min_phase") = 0)
		.def("GetNbCycles", &SegmentationLayer::GetNbCycles)
		.def("GetNbCascades", &SegmentationLayer::GetNbCascades)
		.def("GetNbSpikes", &SegmentationLayer::GetNbSpikes);

	py::class_<PixelLayer, SegmentationLayer>(m, "PixelLayer")
		.def(py::init<const string&>())
		.def(py::init<const cv::Mat&>());
	//	.def("Add", &SensorPixel::DebugSegmentation)
	//	.def("SetWorkingDir", &SensorPixel::SetWorkingDir);

	py::class_<SegmentLayerMonitor>(m, "SegLayerMonitor")
		.def(py::init<string, SegmentationLayer&>())
		.def("GetDisplay", &SegmentLayerMonitor::GetDisplay);

	
}
