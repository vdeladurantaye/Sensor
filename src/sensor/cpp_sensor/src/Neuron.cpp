/** @file Neuron.cpp
 *
 *
 *  @author Vincent de Ladurantaye
 *  @date 02/2010
 */

#include <cmath>

#include "Neuron.h"

//=============================================================================
//									  Neuron
//=============================================================================
//=============================================================================
Neuron::Neuron()
	:
	pot(0.0f),
	phase(-1),
	max_charge(0.0f),
	id(-1),
	pos(-1, -1),
	label(-1),
	nb_spikes(0),
	cycle_spiked(false),
	is_segmented(false), 
	last_spike(0),
	fire_period(0),
	delta_period(-1)
{
	
}

//=============================================================================
void Neuron::Spike(int a_phase, float a_sim_time)
{
	float tmpPeriod; // Temporary period value

	if (last_spike > 0)
	{
		tmpPeriod = a_sim_time - last_spike;
		delta_period = fire_period - tmpPeriod;
		fire_period = tmpPeriod;
		last_spike = a_sim_time;
	}
	else
	{
		last_spike = a_sim_time;
	}

	// Set potential to big negative value so the neuron doesn't spike
	// again in this wave. The potential will be reset to 0 after the spike
	// cascade by NeuralLayer::GlobalInhibition()
	pot = -100000;

	// Set the new phase
	phase = a_phase;

	++nb_spikes;
	cycle_spiked = true;
}

//=============================================================================
bool Neuron::operator==(const Neuron& n) const
{
	if (pot != n.pot
		|| phase != n.phase
		|| max_charge != n.max_charge
		|| id != n.id
		|| label != n.label
		|| nb_spikes != n.nb_spikes
		|| cycle_spiked != n.cycle_spiked
		|| is_segmented != n.is_segmented) return false;

	return true;
}

//=============================================================================
bool Neuron::operator!=(const Neuron& n) const
{

	return !(*this == n);
}


