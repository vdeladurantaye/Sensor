/**
* @file NeuralLayer.h
*
* @authors Vincent de Ladurantaye
*/
#pragma once

#include "NeuralLayer.h"

/**
* Neuron Relative Position, describing the position of a neuron relative to
* another. Used for weights lookup table
*/
// Todo, rename without the N_ prefix so. To do so we have to remove references
// to the other enum that defines the same terms
enum NeuronRelPos
{
	N_LEFT = 0,
	N_RIGHT = 1,
	N_UP = 2,
	N_DOWN = 3,
	N_UP_R = 4,
	N_DOWN_L = 5,
	N_UP_L = 6,
	N_DOWN_R = 7
};

struct Segment
{
	int id;
	int phase;
	int nbNeuron;
	int perimeter;
};


//=============================================================================
//								SegmentationLayer
//=============================================================================
/**
* Layer that does ODLM inspired segmentation. Neurons are connected to their
* immediate neighbors.
*/
class SegmentationLayer : public NeuralLayer
{
public:

	/**
	* Constructor
	*/
	SegmentationLayer(const cv::Mat& a_img);
	SegmentationLayer(ImageData& a_img_data);

	/**
	* Destructor
	*/
	virtual ~SegmentationLayer();
	   
	/**
	* Segments the layer until the network converges or until the number
	* of cycles/cascades set in the params is reached
	*/
	virtual void SegmentLayer();

	/**
	* Removes segments that are too small
	*/
	void CountSegments();

	/**
	* Removes segments that are too small
	*/
	void ClearSmallSegments();

	/**
	* Removes segments that are too small
	*/
	//cv::Mat GetSegmentsImg();
	cv::Mat GetImg();

public:

	// List of segments
	vector<Segment> segments;

protected:

	/**
	* Calculates the weights between two neurons in the layer
	*/
	virtual float ComputeWeigth(float a_feat_diff);

	/**
	* Calculates the weights between two adjacent neurons based on their
	* relative position.
	*/
	virtual float ComputeWeigth(int a_src_id, int a_dst_id, 
								NeuronRelPos a_dst_pos) = 0;

	/**
	* Function called by FireNeurons() to propagate a spike to neighboring
	* neurons on the same layer.
	*/
	void PropagateSpike(int a_id, int a_phase);

	/**
	* Propagate a spike to a neighboring neuron
	*/
	void Propagate(int a_src_id, NeuronRelPos a_dst_pos, int a_phase);

	/**
	* Propagate a label to a neuron and merge the segments if necessary
	*/
	virtual void PropagateLabel(Neuron& a_n, int a_label, int a_phase);

	/**
	* Merge two segments by giving the first segment's label to the second
	* segment.
	*/
	void MergeSegments(int a_src_label, int a_dst_label, int a_phase);


	void TriggerSameLabelNeurons(int a_id, int a_phase);

protected:

	// Lookup Table for index offset based on neurons relative positions
	int pos_offset_[8];


	//-----------------------------------------------------------------------------
	//							 Layer public parameters
	//-----------------------------------------------------------------------------
public:
	uint MAX_SEG_CASCADES;
	uint MAX_SEG_CYCLES;
	uint MIN_SEGMENT_SIZE;

	bool TRIGGER_SAME_LABEL_NEURONS;
	bool MERGE_SEGMENTS;
	float SEG_MERGE_TRESHOLD;

	float WEIGHT_MAX_VALUE;
	float WEIGHT_SLOPE;
	float WEIGHT_OFFSET;

};
