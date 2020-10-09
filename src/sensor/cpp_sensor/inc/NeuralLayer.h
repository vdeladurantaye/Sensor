/**
* @file NeuralLayer.h
*
* @authors Vincent de Ladurantaye
*/
#pragma once

#include "Neuron.h"
#include "ImageData.h"


/**
* Abstract class for a layer of spiking neurons. Different implementations are
* required for different type of neuron features.
*/
class NeuralLayer
{
public:

	/**
	* Constructor
	*/
	NeuralLayer(const ImageData& a_data, int a_layer_id = -1);

	/**
	* Virtual Destructor since this is an abstract class
	*/
	virtual ~NeuralLayer();


	/**
	* Find the neuron with the highest potential and returns the simulation
	* time necessary to make it spike
	*/
	float FindNextTimeStep();

	/**
	* Advances time for all neurons on layer
	*
	* @param a_delta Amount of time to add to the simulation
	*/
	void AdvanceTime(float a_delta);

	/**
	* Fires the neurons with potential above the threshold.
	*
	* @param a_phase Phase of this firing episode, represented in the original
	*	ODLM code by the counter of firing cascade
	* @param a_sim_time Simulation time of the simulation, this is used to
	*	calculate the network convergence.
	*/
	int FireNeurons(int a_phase, float a_sim_time);

	/**
	* Check if a complete cycle of spiking has occured in the network. A cycle
	* is completed when all leader neurons have fired at least once.
	*/
	bool IsCycleCompleted();

	/**
	* Reset neuron's cycle spike flag for the next cycle.
	*/
	void ResetCycle();

	/**
	* Reduces the potential of the neurons that didn't fire
	*/
	virtual void GlobalInhibition();

	/**
	* Get the stabilization coefficient indicating if the network has 
	* converged and neurons are firing in stable intervals.
	*
	* @param a_min_phase Minimum phase number to be considered. This is used
	*	to avoid taking into account neurons that haven't fired.
	*/
	float GetCoefStabilization(int a_min_phase = 0);

	/**
	* Set the callback for propagating the spike out of the layer.
	*/
	void SetPropagateCallback(function< void(uint, uint, uint) > a_callback)
	{
		PropagateSpikeOutOfLayer = a_callback;
	}

	/**
	* Save the layer's neuron state to file
	*/
	void SaveStateToFile(string a_filename);

	/**
	* Compares the layer to the state saved in the given file. Returns true if
	* the layer is identical.
	*/
	bool ValidateLayerState(string a_filename);
	
	/**
	* Check if the given position is within the layer
	*/
	inline bool IsInLayer(int a_x, int a_y)
	{
		return  (a_x >= 0 &&
				 a_y >= 0 &&
				 a_x < (int)width &&
				 a_y < (int)height);
	}

	/// Get the number of cycles
	uint GetNbCycles() { return n_cycles; }
	/// Get the number of cascades
	uint GetNbCascades() { return n_cascades; }
	/// Get the number of spikes
	unsigned long GetNbSpikes() { return n_spikes; }

public:

	// Vector of neurons
	vector<Neuron> neurons;

	// Width of the layer
	uint width;
	// Height of the layer
	uint height;
	// Size of the layer (width*height). Declared as a separate member for 
	// readability and convenience.
	uint size;

	// Id of the layer
	uint layer_id;

protected:
	
	/**
	* Calculates the weights between two neurons in the layer
	*/
	virtual float ComputeWeigth(float a_feat_diff) = 0;

	/**
	* Function called by FireNeurons() to propagate a spike to neighboring
	* neurons on the same layer.
	*/
	virtual void PropagateSpike(int a_id, int a_phase) = 0;

	/**
	* Propagate a new label to a neuron.
	*/
	virtual void PropagateLabel(Neuron& a_n, int a_label, int a_phase) = 0;

	// Callback to propagate spikes to other layers
	function< void(uint neuron_id, uint layer_id, uint phase) >
		PropagateSpikeOutOfLayer;

protected:

	// Image data represented by this layer
	ImageData img_data_;

	// Region where neurons neurons are processed. We have this so that we can
	// concentrate on specific parts of the layer and ignore the rest.
	cv::Rect active_reg_;

	// Simulation time
	float sim_time;
	// Cycle counter
	uint n_cycles;
	// Cascade counter
	uint n_cascades;
	// Spike counter
	unsigned long n_spikes;


	// Static counter to give a unique ID to each layer
	static uint layer_id_counter_;

	// Static label counter for giving neurons unique labels throughout neurons
	// of all layers
	static uint label_counter_;


//-----------------------------------------------------------------------------
//							Configuration Parameters
//-----------------------------------------------------------------------------
public:

	float POT_THRESHOLD;
	float TAU;
	float GLOBAL_INHIB_VAL;
	float CHARGING_LEADER;
	float CHARGING_FOLLOW;

public:
	friend class LayerDebugger;
	friend class LayerCoupler;
	friend class LayerMonitor;

};
