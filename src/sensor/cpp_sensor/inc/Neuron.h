/** @file Neuron.h
 *
 *  @author Vincent de Ladurantaye
 */

#pragma once

#include "Tools.h"


/**
* Integrate and fire neuron for ODLM type neural networks. Spikes when the
* neural potential threshold is reached and propagates it's label. Each neuron
* represents some type of feature(s).
*
*/
class Neuron
{
public:

	/**
	* Constructor
	*/
	Neuron();

	void Spike(int a_phase, float a_sim_time);

	/**
	* Comparison operator
	*/
	bool operator==(const Neuron& n) const;
	bool operator!=(const Neuron& n) const;

public:

	// Neuron membrane potential
	float pot;

	// Last cascade number where the neuron fired
	int phase;

	// Maximum potential the neuron can reach. This can exceed the threshold to
	// make a leader neuron that spikes on it's own.
	float max_charge;

	// Index of the neuron in the layer
	uint id;

	// Position of the neuron in the layer
	cv::Point pos;

	// Label used to identify segments
	int label;

	// Counter for the number of times the neuron has fired
	uint nb_spikes;

	// Indicates wether the neuron has fired in the current cycle or not
	bool cycle_spiked;

	// Indicates if neuron is part of a segment or not
	bool is_segmented;
        
	// Time of last spike
	float last_spike;
	// Period between last two spikes
	float fire_period;
	// Variation in firing period
	float delta_period;

};

