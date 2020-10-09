/** @file Sensor_python.h
 * Wrapper for using the C++ code from Python. 
 * This is done using pybind11: https://github.com/pybind/pybind11

 *
 * @author Vincent de Ladurantaye
 */

#include "Tools.h"
#include "SegmentationLayer.h"


/** 
* Python wrapper for the NeuralLayer class, which is abstract. Pybind11 will 
* use this class as "trampoline" if we try to instanciate a SegmentationLayer
* from python.
*/
class PySegLayer: public SegmentationLayer
{
public:
	/* Inherit the constructors */
	using SegmentationLayer::SegmentationLayer;

	/* Trampoline (need one for each virtual function) */
	float ComputeWeigth(int a_src_id, int a_dst_id,
				  NeuronRelPos a_dst_pos) override {return 0.0f;}

};
