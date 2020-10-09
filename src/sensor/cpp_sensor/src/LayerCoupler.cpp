/** @file MatchingNetwork.cpp
*
*
*  @author Vincent de Ladurantaye
*/

#include "LayerCoupler.h"

//=============================================================================
//								LayerCoupler
//=============================================================================
LayerCoupler::LayerCoupler(NeuralLayer* layer1, NeuralLayer* layer2):
	layers_{layer1,layer2},
	WEIGHT_MAX_VALUE(Config::MATCHING_WEIGHT_MAX),
	WEIGHT_SLOPE(Config::MATCHING_WEIGHT_SLOPE),
	WEIGHT_OFFSET(Config::MATCHING_WEIGHT_OFFSET)
{
	layers_[L1]->SetPropagateCallback(
		[&] (uint id, uint layer_id, uint phase) 
		{ this->Layer1SpikeHandler(id, layer_id, phase); });

	layers_[L2]->SetPropagateCallback(
		[&] (uint id, uint layer_id, uint phase) 
		{ this->Layer2SpikeHandler(id, layer_id, phase); });
}

//=============================================================================
LayerCoupler::~LayerCoupler()
{
}

//=============================================================================
float LayerCoupler::ComputeWeigth(uint idLayer1, uint idLayer2)
{
	return 1 - 1/(1 + exp(-WEIGHT_SLOPE * 
				  (ComputeFeatDiff(idLayer1, idLayer2) - WEIGHT_OFFSET)));
}

//=============================================================================
void LayerCoupler::PropagateSpikeL1toL2(
	const Neuron& n1,
	Neuron& n2,
	int phase)
{	
	n2.pot += WEIGHT_MAX_VALUE * ComputeWeigth(n1.id, n2.id);

	layers_[L2]->PropagateLabel(n2, n1.label, phase);
}

//=============================================================================
void LayerCoupler::PropagateSpikeL2toL1(
	const Neuron& n2,
	Neuron& n1,
	int phase)
{
	n1.pot += WEIGHT_MAX_VALUE * ComputeWeigth(n1.id, n2.id);

	layers_[L1]->PropagateLabel(n1, n2.label, phase);
}


//=============================================================================
//								PixelLayerCoupler
//=============================================================================
PixelLayerCoupler::PixelLayerCoupler(
	PixelLayer* inLayer,
	PixelLayer* refLayer)
	:
	LayerCoupler(inLayer, refLayer)
{
}

//=============================================================================
void PixelLayerCoupler::Layer1SpikeHandler(uint neuron_id, 
										   uint layer_id,
										   uint phase)
{
	for (auto& n: layers_[L2]->neurons)
	{
		PropagateSpikeL1toL2(layers_[L1]->neurons[neuron_id], n, phase);
	}
}

//=============================================================================
void PixelLayerCoupler::Layer2SpikeHandler(uint neuron_id, 
										   uint layer_id, 
										   uint phase)
{
	for (auto& n: layers_[L1]->neurons)
	{
		PropagateSpikeL2toL1(layers_[L2]->neurons[neuron_id], n, phase);
	}
}

//=============================================================================
float PixelLayerCoupler::ComputeFeatDiff(uint idLayer1, uint idLayer2)
{
	return abs( (float)
		static_cast<PixelLayer*>(layers_[L1])->pixel_data[idLayer1] -
		static_cast<PixelLayer*>(layers_[L2])->pixel_data[idLayer2]);
}

