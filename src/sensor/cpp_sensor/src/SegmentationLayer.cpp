/** @file NeuralLayer.cpp
*
*
*  @author Vincent de Ladurantaye
*/

#include "SegmentationLayer.h"
#include "LayerDebugger.h"

#include <iostream>
#include <fstream>
using namespace std;

//=============================================================================
//								 SegmentationLayer
//=============================================================================
SegmentationLayer::SegmentationLayer(const cv::Mat& a_img) :
	SegmentationLayer(ImageData(a_img))
{
}
//-----------------------------------------------------------------------------

SegmentationLayer::SegmentationLayer(ImageData& a_img_data) :
	NeuralLayer(a_img_data),
	MAX_SEG_CASCADES(Config::SEG_MAX_CASCADES),
	MAX_SEG_CYCLES(Config::SEG_MAX_CYCLES),
	MIN_SEGMENT_SIZE(Config::MIN_SEGMENT_SIZE),
	TRIGGER_SAME_LABEL_NEURONS(Config::SEG_TRIGGER_SAME_LABEL_NEURONS),
	MERGE_SEGMENTS(Config::SEG_MERGE_SEGMENTS),
	WEIGHT_MAX_VALUE(Config::SEG_WEIGHT_MAX),
	WEIGHT_SLOPE(Config::SEG_WEIGHT_SLOPE),
	WEIGHT_OFFSET(Config::SEG_WEIGHT_OFFSET)
{
	SEG_MERGE_TRESHOLD = ComputeWeigth(Config::SEG_MERGE_DELTA);

	pos_offset_[N_UP_L] = -(int)width - 1; // using type cast to avoid warning:
	pos_offset_[N_UP] = -(int)width;	   // minus applied to unsigned type
	pos_offset_[N_UP_R] = -(int)width + 1;
	pos_offset_[N_LEFT] = -1;
	pos_offset_[N_RIGHT] = +1;
	pos_offset_[N_DOWN_L] = width - 1;
	pos_offset_[N_DOWN] = width;
	pos_offset_[N_DOWN_R] = width + 1;

}

//=============================================================================
SegmentationLayer::~SegmentationLayer()
{
}

//=============================================================================
void SegmentationLayer::SegmentLayer()
{
	int stableCascadeCount = 0;
	float stabilizationCoef = 0.0f;
	while (n_cycles < MAX_SEG_CYCLES)
	{

#ifdef LAYER_DEBUGGER
		LayerDebugger::SetBreakpoint(*this, DEBUG_LEVEL_CASCADE,
									 n_cascades);
#endif
		float delta = FindNextTimeStep();

		sim_time += delta;

		AdvanceTime(delta);

		while (FireNeurons(n_cascades, sim_time) > 0)
		{
		}

		GlobalInhibition();

		++n_cascades;

		// Check if spikes are stable
		stabilizationCoef = GetCoefStabilization(0);
		if (stabilizationCoef < 0.4)
		{
			++stableCascadeCount;
		}
		else stableCascadeCount = 0;

		// If enough consecutive cascades were stable, stop the simulation
		if (stableCascadeCount >= 1)
		{
			break;
		}

		// If we have a max number of cascade to do, check if we reached it
		if (MAX_SEG_CASCADES > 0 && n_cascades >= MAX_SEG_CASCADES) break;

		// Check if cycle is completed
		if (IsCycleCompleted() == false)
		{
			// If not, continue this cycle
			continue;
		}

#ifdef LAYER_DEBUGGER
		LayerDebugger::SetBreakpoint(*this, DEBUG_LEVEL_CYCLE, n_cycles);
#endif

		++n_cycles;
		ResetCycle();
	}
	
	//ClearSmallSegments();

	cout << "\nCycle: " << n_cycles << "\tCascade: " << n_cascades
		<< "\tSpikes: " << n_spikes 
		<< "\tConvergence: " << stabilizationCoef<< endl;

#ifdef LAYER_DEBUGGER
	LayerDebugger::SetBreakpoint(*this, DEBUG_LEVEL_END);
#endif
}

//=============================================================================
void SegmentationLayer::CountSegments()
{
	segments.clear();

	vector<int> segmentCounts(size);

	for (uint i = 0; i < size; ++i)
	{
		segmentCounts[i] = 0;
	}

	// Iterate through all neurons to count neurons with the same labels
	for (auto& neuron : neurons)
	{
		// If the phase is higher than 0, we have a neuron part of a segment
		if (neuron.phase > 0)
		{
			// Check if the segment already exist
			bool segmentFound = false;
			for (auto& segment : segments)
			{
				if (segment.id == neuron.label)
				{
					++segment.nbNeuron;
					segmentFound = true;
				}
			}

			// If we didn't find the segment, create it
			if (segmentFound == false)
			{
				Segment seg;
				seg.id = neuron.label;
				seg.phase = neuron.phase;
				seg.nbNeuron = 1;

				segments.push_back(seg);
			}
		}
	}

	cout << "Number of segments: " << segments.size() << endl;
}

//=============================================================================
void SegmentationLayer::ClearSmallSegments()
{
	if (segments.empty()) CountSegments();

	// Iterate through segments and set to 0 the phase of neurons part of 
	// small segments so that they have the same phase as neurons
	// that didn't fire.
	for (auto& segment : segments)
	{
		if (segment.nbNeuron < MIN_SEGMENT_SIZE)
		{
			for (auto& neuron : neurons)
			{
				if (neuron.label == segment.id)
				{
					neuron.phase = 0;
				}
			}
		}
	}
}

//=============================================================================
cv::Mat SegmentationLayer::GetImg()
{
	cout << "TEST: "
		<< "rows: " << img_data_.image_.rows << " "
		<< "cols: " << img_data_.image_.cols << endl;
	return img_data_.image_.clone();
}

//=============================================================================
//								Protected
//=============================================================================
float SegmentationLayer::ComputeWeigth(float a_feat_delta)
{
	//return WEIGHT_MAX_VALUE * exp(-WEIGHT_SLOPE * a_feat_delta);

	float deltaCoef = 1 - 1 / (1 + exp(-WEIGHT_SLOPE * (abs(a_feat_delta) -
														WEIGHT_OFFSET)));
	return WEIGHT_MAX_VALUE * deltaCoef;
}

//=============================================================================
void SegmentationLayer::PropagateSpike(int a_id, int a_phase)
{
	// Calculate the row and column of the current index
	int neuronRow = neurons[a_id].pos.y;
	int neuronCol = neurons[a_id].pos.x;

	//-------------------------------------------------------------------------
	// Propagate the spike among neirghbors
	//-------------------------------------------------------------------------
	// Add weights to neighbors and propagate the neuron ID
	if (neuronRow > 0 && neuronCol > 0)
	{
		Propagate(a_id, N_UP_L, a_phase);
	}

	if (neuronRow > 0)
	{
		Propagate(a_id, N_UP, a_phase);
	}

	if (neuronRow > 0 && neuronCol < width - 2)
	{
		Propagate(a_id, N_UP_R, a_phase);
	}

	if (neuronCol > 0)
	{
		Propagate(a_id, N_LEFT, a_phase);
	}

	if (neuronCol < width - 2)
	{
		Propagate(a_id, N_RIGHT, a_phase);
	}

	if (neuronRow < height - 2 && neuronCol > 0)
	{
		Propagate(a_id, N_DOWN_L, a_phase);
	}

	if (neuronRow < height - 2)
	{
		Propagate(a_id, N_DOWN, a_phase);
	}

	if (neuronRow < height - 2 && neuronCol < width - 2)
	{
		Propagate(a_id, N_DOWN_R, a_phase);
	}

	//-------------------------------------------------------------------------
	// Trigger neurons with the same label
	//-------------------------------------------------------------------------
	if (TRIGGER_SAME_LABEL_NEURONS) TriggerSameLabelNeurons(a_id, a_phase);

}

//=============================================================================
void SegmentationLayer::Propagate(int a_src_id,
								  NeuronRelPos a_dst_pos,
								  int a_phase)
{
	Neuron& n1 = neurons[a_src_id];
	Neuron& n2 = neurons[a_src_id + pos_offset_[a_dst_pos]];

	// Return immediatly if both neuron have the same label as they fire 
	// together anyway, so the destination neuron is sure to fire, no point in
	// increasing the potential
	if (TRIGGER_SAME_LABEL_NEURONS && n1.label == n2.label) return;

	// Add the weight to the potential
	float w = ComputeWeigth(a_src_id, n2.id, a_dst_pos);
	n2.pot += w;

	// Don't propagate receiving neuron isn't over the threshold
	if (n2.pot < POT_THRESHOLD) return;

	// If already the same label, no need to propagate
	if (n1.label == n2.label) return;

	// If the connection strengh is strong enough, merge the segments
	if (MERGE_SEGMENTS && n2.is_segmented && w > SEG_MERGE_TRESHOLD)
		MergeSegments(n1.label, n2.label, a_phase);

	PropagateLabel(n2, n1.label, a_phase);

}

//=============================================================================
void SegmentationLayer::PropagateLabel(Neuron& a_n, int a_label, int a_phase)
{
	// Propagate the label to this neuron
	a_n.label = a_label;
	// Set the new phase
	a_n.phase = a_phase;
	// Set as part of a segment
	a_n.is_segmented = true;
}

//=============================================================================
void SegmentationLayer::MergeSegments(int a_src_label, 
									  int a_dst_label, 
									  int a_phase)
{
	// Make all the neurons with the destination neuron label fire
	// Iterate through all neurons
	for (int y = active_reg_.y; y < active_reg_.height; ++y) for
		(int x = active_reg_.x; x < active_reg_.width; ++x)
	{
		int i = y * width + x;

		// If it has the destination neuron label
		if (neurons[i].label == a_dst_label)
		{
			// Rise the potential to the firing threshold
			neurons[i].pot = POT_THRESHOLD;
			// Set their new label
			neurons[i].label = a_src_label;
			// Set the new phase
			neurons[i].phase = a_phase;
		}
	}
}

//=============================================================================
void SegmentationLayer::TriggerSameLabelNeurons(int a_id, int a_new_phase)
{
	// Check if the phase is the same as the new phase and skip if so.
	if (neurons[a_id].phase == a_new_phase) return;
	// If the phase is different from the new phase, this means this neuron is
	// the first of his segment to spike. After this function is done, all 
	// neurons with the same label as this neuron will be set to the new phase
	// and will thus skip this function executing it only once per segment.

	// Trigger all neurons with same ID
	for (int y = active_reg_.y; y < active_reg_.height; ++y) for 
		(int x = active_reg_.x; x < active_reg_.width; ++x)
	{
		int i = y * width + x;
		Neuron& tmpN = neurons[i];

		if (i != a_id // If not the current neuron
			&& tmpN.label == neurons[a_id].label // And has same label
			&& tmpN.phase != a_new_phase) // And hasn't fired yet
		{
			// Rise the potential to the firing threshold
			tmpN.pot = POT_THRESHOLD;
			// Set their new Phase now so it doesn't redo this 
			// loop when the neuron fires.
			tmpN.phase = a_new_phase;
		}
	}
}
