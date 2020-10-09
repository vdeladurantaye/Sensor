/** @file NeuralLayer.cpp
*
*
*  @author Vincent de Ladurantaye
*/

#include "NeuralLayer.h"
#include "LayerDebugger.h"

#include <iostream>
#include <fstream>
using namespace std;

//=============================================================================
//						Static members declarations
//=============================================================================
uint NeuralLayer::layer_id_counter_ = 0;
uint NeuralLayer::label_counter_ = 0;

//=============================================================================
//									NeuralLayer
//=============================================================================
NeuralLayer::NeuralLayer(const ImageData& a_data, int a_layer_id):
	width(a_data.cols),
	height(a_data.rows),
	size(a_data.size),
	layer_id(a_layer_id),
	img_data_(a_data),
	active_reg_(0, 0, 0, 0),
	sim_time(0.0f),
	n_cycles(0),
	n_cascades(0),
	n_spikes(0),
	POT_THRESHOLD(Config::POT_THRESHOLD),
	TAU(Config::TAU),
	GLOBAL_INHIB_VAL(Config::GLOBAL_INHIB_VAL),
	CHARGING_LEADER(Config::CHARGING_LEADER),
	CHARGING_FOLLOW(Config::CHARGING_FOLLOWER)
{
	if (a_layer_id == -1) layer_id = layer_id_counter_++;

	// Set the layer size
	width = a_data.cols;
	height = a_data.rows;
	size = a_data.size;

	// Set the layer active region as the whole layer
	active_reg_.x = 0;
	active_reg_.y = 0;
	active_reg_.width = width;
	active_reg_.height = height;

	// Create the neurons using std::vector::assign()
	neurons.assign(width*height, Neuron());
	for (int i = 0; i < size; ++i)
	{
		neurons[i].id = i;
		neurons[i].pos.x = i % width;
		neurons[i].pos.y = i / width;
		neurons[i].label = label_counter_++;
	}

}

//=============================================================================
NeuralLayer::~NeuralLayer()
{
}

//=============================================================================
float NeuralLayer::FindNextTimeStep()
{
	int id;
	float max = 0;
	// Iterate through all neurons
	for (auto& n: neurons)
	{
		// Find the max neuron that has a charging potential greater than the 
		// threshold
		if (n.max_charge > POT_THRESHOLD && n.pot > max)
		{
			max = n.pot;
			id = n.id;
		}
	}

	if (max >= POT_THRESHOLD) return 0;

	// Calculate the charging time statically to calculate it only once. 
	// Charging time is the time needed for a neuron with 0 potential to spike.
	static float chargingTime = TAU * log(CHARGING_LEADER
										  / (CHARGING_LEADER - POT_THRESHOLD));
	// Return the time needed for the max neuron to spike
	return chargingTime - TAU * log(CHARGING_LEADER / (CHARGING_LEADER -max));
}

//=============================================================================
void NeuralLayer::AdvanceTime(float a_delta)
{
	if (a_delta == 0) return;

	// Calculate the exponential of delta before the loop
	float expDelta = exp(-a_delta /TAU);

	// Iterate through all neurons
	for (int y=active_reg_.y; y<active_reg_.height; ++y) for 
		(int x=active_reg_.x; x<active_reg_.width; ++x)
	{
		int i = y*width + x;

		// If the potential is negative, set it to 0
		if (neurons[i].pot < 0) neurons[i].pot = 0;

		// Set the new potential
		neurons[i].pot = neurons[i].max_charge - expDelta *
			(neurons[i].max_charge - neurons[i].pot);
	}
}

//=============================================================================
int NeuralLayer::FireNeurons(int a_phase, float a_sim_time)
{
	int spikeCount = 0; // Counter for the number of spikes

	// Iterate through all neurons in the active region of the layer
	for (int y=active_reg_.y; y<active_reg_.height; ++y)
	for (int x=active_reg_.x; x<active_reg_.width; ++x)
	{
		int i = y*width + x;

		Neuron& neuron = neurons[i];
		// If the potential is above the threshold
		if (neuron.pot >= POT_THRESHOLD)
		{
			// Increment the number of spikes
			spikeCount++;

			// Propagate the spike within the layer
			PropagateSpike(i, a_phase);

			// Propagate the spike out of the layer
			if (PropagateSpikeOutOfLayer) 
				PropagateSpikeOutOfLayer(i, layer_id, a_phase);

			neuron.Spike(a_phase, a_sim_time);

#ifdef LAYER_DEBUGGER
			LayerDebugger::SetBreakpoint(*this, DEBUG_LEVEL_SPIKE, i);
#endif
		}
	}

	n_spikes += spikeCount;
	return spikeCount;
}

//=============================================================================
bool NeuralLayer::IsCycleCompleted()
{
	// Iterate through all neurons
	for (int y=active_reg_.y; y<active_reg_.height; ++y)
	for (int x=active_reg_.x; x<active_reg_.width; ++x)
	{
		int i = y*width + x;

		if (neurons[i].max_charge == CHARGING_LEADER
			&& neurons[i].cycle_spiked == false)
			return false;
	}

	return true;
}

//=============================================================================
void NeuralLayer::ResetCycle()
{
	for (int i=0; i<size; ++i)
	{
		neurons[i].cycle_spiked = false;
	}
}

//=============================================================================
void NeuralLayer::GlobalInhibition()
{
	// Iterate through all neurons
	for (int y=active_reg_.y; y<active_reg_.height; ++y)
	for (int x=active_reg_.x; x<active_reg_.width; ++x)
	{
		int i = y*width + x;

		// Inhibate the neuron that didn't fire
		if (neurons[i].pot > 0) neurons[i].pot -= GLOBAL_INHIB_VAL;

		// If the potential is negative, set it to 0
		if (neurons[i].pot < 0) neurons[i].pot = 0;
	}
}

//=============================================================================
float NeuralLayer::GetCoefStabilization(int a_min_phase)
{
	double sumDP; // Sum of all deltaPeriod for regions higher than minRegion
	int    qtyDP; // How many deltaPeriod added

	sumDP = 0.0;
	qtyDP = 0;

	// Iterate through all neurons
	for (int y = active_reg_.y; y < active_reg_.height; ++y)
	for (int x = active_reg_.x; x < active_reg_.width; ++x)
	{
		int i = y * width + x;

		Neuron& neuron = neurons[i];

		if (neuron.phase > a_min_phase)
		{
			qtyDP++;
			sumDP += fabs(neuron.delta_period);
		}
	}

	if (qtyDP > 0)
	{
		return (sumDP / qtyDP);
	}
	else
	{
		return (1.0);
	}
}

//=============================================================================
void NeuralLayer::SaveStateToFile(string a_filename)
{
	// Open the output file
	ofstream outFile(a_filename);
	// Check if we were able to open it
	if (!outFile)
	{
		cerr << "Failed open file " << a_filename << '\n';
		return;
	}

	outFile << "id" << '\t' << "label" << '\t' << "potential" << endl;

	for (auto& n: neurons)
	{
		outFile << n.id << '\t' << n.label << '\t' << n.pot << endl;
	}

	outFile.close();
}

//=============================================================================
bool NeuralLayer::ValidateLayerState(string a_validationFilename)
{
	// Open the validation file
	ifstream inFile(a_validationFilename);

	// Check if we were able to open it
	if (!inFile)
	{
		cerr << "Failed to load validation file: " << a_validationFilename
			<< '\n';
		return false;
	}

	// Get the first line which doesn't contain neuron data
	string line;
	getline(inFile, line);

	int id, label;
	float pot;
	for (auto& n : neurons)
	{
		//getline(inFile, line);
		inFile >> id >> label >> pot;

		if (id != n.id || label != n.label || abs(pot - n.pot) > 0.0005f)
		{
			return false;
		}
	}

	return true;
}
