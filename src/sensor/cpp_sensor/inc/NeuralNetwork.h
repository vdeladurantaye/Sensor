/**
* NeuralNetwork.h
*
* @authors Vincent de Ladurantaye
*/
#pragma once

#include "PixelLayer.h"
#include "LayerCoupler.h"

/**
* Abstract class representing a neural network. A network is meant
*/
class NeuralNetwork
{
public:

	/**  Constructor
	*/
	NeuralNetwork();

	/**  Destructor
	*/
	virtual ~NeuralNetwork();

	/**
	* Runs the network.
	*/
	virtual void Run() = 0;
	
};


class PixelODLM : public NeuralNetwork
{
public:

	/**  Constructor
	*/
	PixelODLM(const string& inputImageName,
			  const string& refImageName);
	
	/**
	* Runs the network.
	*/
	virtual void Run();


private:

	ImageData input_data_;
	ImageData ref_data_;

	// Input layer
	PixelLayer input_layer_;
	// Reference layer
	PixelLayer ref_layer_;

	PixelLayerCoupler layer_coupler_;


	// Processing threads
	//std::thread input_thread_;
	//std::thread ref_thread_;

};

