/**
* LayerCoupler.h
*
* @authors Vincent de Ladurantaye
*/
#pragma once

#include "PixelLayer.h"

#include <array>


/** @class LayerCoupler
* Handles the communications between 2 neural layers. This allows 
* decoupling the layers so they don't reference each others.
*/
class LayerCoupler
{
public:

	enum LayersId
	{
		L1,
		L2
	};

	/**  
	* Constructor
	*/
	LayerCoupler(NeuralLayer* layer1, NeuralLayer* layer2);

	/**
	* Destructor
	*/
	virtual ~LayerCoupler();

	virtual void Layer1SpikeHandler(
		uint neuron_id, uint layer_id, uint phase) = 0;

	virtual void Layer2SpikeHandler(
		uint neuron_id, uint layer_id, uint phase) = 0;


//	virtual array<vector<Point>, 2> GetMatchingPoints() = 0;
	
protected:

	/**
	* Calculates the weights between two neurons in the layer
	*/
	virtual float ComputeWeigth(uint idLayer1, uint idLayer2);

	/**
	* Calculates the absolute value between the features of the neurons on both
	* layers.
	*/
	virtual float ComputeFeatDiff(uint idLayer1, uint idLayer2) = 0;

	/**
	* Propagate a spike from layer 1 to layer 2
	*/
	void PropagateSpikeL1toL2(
		const Neuron& n1,
		Neuron& n2, 
		int phase);

	/**
	* Propagate a spike from layer 2 to layer 1
	*/
	void PropagateSpikeL2toL1(
		const Neuron& n2,
		Neuron& n1,
		int phase);
		
protected:

	array<NeuralLayer*, 2> layers_;


//------------------------------------------------------------------------------
//							Configuration Parameters
//------------------------------------------------------------------------------
protected:

	float WEIGHT_MAX_VALUE;
	float WEIGHT_SLOPE;
	float WEIGHT_OFFSET;

	float SEG_MERGE_TRESHOLD;

};


//=============================================================================
//								PixelLayerCoupler
//=============================================================================
class PixelLayerCoupler: public LayerCoupler
{
public:

	/**  
	* Constructor
	*/
	PixelLayerCoupler(PixelLayer* inLayer, PixelLayer* refLayer);

	/** 
	* Destructor
	*/
	virtual ~PixelLayerCoupler() {}

	virtual void Layer1SpikeHandler(
		uint neuron_id, uint layer_id, uint phase);

	virtual void Layer2SpikeHandler(
		uint neuron_id, uint layer_id, uint phase);

	/**
	* Calculates the absolute value between the pixel of each neuron
	*/
	virtual float ComputeFeatDiff(uint idLayer1, uint idLayer2);
	
};
