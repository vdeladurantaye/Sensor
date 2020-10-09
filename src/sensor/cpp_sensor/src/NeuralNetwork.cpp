/**
*
*  @author Vincent de Ladurantaye
*/


#include "NeuralNetwork.h"

#include "Monitor.h"
#include "LayerDebugger.h"


#include <thread>
using namespace std;

//=============================================================================
NeuralNetwork::NeuralNetwork()
{
}

//=============================================================================
NeuralNetwork::~NeuralNetwork()
{
}

//=============================================================================
//									ClassicalODLM
//=============================================================================
//=============================================================================
PixelODLM::PixelODLM(const string& inputImageName,
					 const string& refImageName) :
	input_data_(inputImageName),
	ref_data_(refImageName),
	input_layer_(input_data_),
	ref_layer_(ref_data_),
	layer_coupler_(&input_layer_, &ref_layer_)
{
}

//=============================================================================
void PixelODLM::Run()
{
	input_layer_.SegmentLayer();
	ref_layer_.SegmentLayer();

	//#ifndef __APPLE__
	//	thread inputThread(&PixelLayer::SegmentLayer, &input_layer_);
	////	thread refThread(&PixelLayer::SegmentLayer, &ref_layer_);
	//
	//	LayerDebugger::WaitForWorkerThreads();
	//
	//	inputThread.join();
	////	refThread.join();
	//#endif

}

