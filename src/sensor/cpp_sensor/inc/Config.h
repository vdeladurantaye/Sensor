/** @file Config.h
 * This file contains parameters controlling different aspect of the
 * simulation
 *
 * @author Vincent de Ladurantaye
 */

#pragma once
#pragma warning(disable: 4244) // possible loss of data (double to float)
#pragma warning(disable: 4267) // possible loss of data (size_t to uint)
#pragma warning(disable: 4018) // '<': signed / unsigned mismatch

#include <string>
using namespace std;

#include <boost/property_tree/ptree.hpp>

//#define __APPLE__

//=============================================================================
//								   Typedefs
//=============================================================================
typedef unsigned int uint;

//=============================================================================
//									Config
//=============================================================================
/**
* Contains all the parameters that can be modified. Parameters are stored as 
* static variables, they are thus global and accessible anywhere in the code.
*/
class Config
{
public:

	/**  Constructor
	*/
	Config()
	{
	}

	//-------------------------------------------------------------------------
	// General configuration parameters
	//-------------------------------------------------------------------------
	// Working directory where the program will look for files. This is used
	// in the python interface so the working directory can be elsewhere than
	// directly in the python directory
	static string WORKING_DIR;
	
	//-------------------------------------------------------------------------
	// Neuron potential parameters
	//-------------------------------------------------------------------------
	// Firing threshold of the neurons
	static float POT_THRESHOLD;
	// Tau in neuron equation
	static float TAU;
	// Global inibition value
	static float GLOBAL_INHIB_VAL;
	// Leader
	static float CHARGING_LEADER;
	// Follower
	static float CHARGING_FOLLOWER;

	//-------------------------------------------------------------------------
	// Neural connexion parameters
	//-------------------------------------------------------------------------
	// Weight constant for segmentation
	static float SEG_WEIGHT_MAX;
	static float SEG_WEIGHT_SLOPE;
	static float SEG_WEIGHT_OFFSET;

	// Weight constant for matching
	static float MATCHING_WEIGHT_MAX;
	static float MATCHING_WEIGHT_SLOPE;
	static float MATCHING_WEIGHT_OFFSET; 

	//-------------------------------------------------------------------------
	// General simulation parameters
	//-------------------------------------------------------------------------
	// Number of maximum segmentation cascades. Set to 0 to run until the net
	static uint SEG_MAX_CASCADES;
	// Number of maximum segmentation cycles
	static uint SEG_MAX_CYCLES;

	// Automaticaly trigger the spiking or all neurons within a segment when a
	// neuron part of the segment spikes
	static bool SEG_TRIGGER_SAME_LABEL_NEURONS;
	// When two neurons of different segments synchronize, automaticly merge
	// their respective segments
	static bool SEG_MERGE_SEGMENTS;
	// Maximal feature divergence for merging segments
	static float SEG_MERGE_DELTA;

	// Minimum number of neurons to have a valid segment
	static uint MIN_SEGMENT_SIZE;

	//-------------------------------------------------------------------------
	// Input Image parameters
	//-------------------------------------------------------------------------
	// Determines if images are resised but keeping their aspect ratio
	static bool RESIZE_IMG_KEEP_RATIO;
	// Lenght of the longest size of the image. The smallest size will be
	// adjusted to preserve the image ratio
	static uint KEEP_RATIO_LONGEST_IMG_SIDE;
	
	// Determines if images are croped or enlarged to a specific size
	static bool FIXED_INPUT_IMGS_SIZE;
	static uint FIXED_INPUT_IMGS_WIDTH;
	static uint FIXED_INPUT_IMGS_HEIGHT;

	//-------------------------------------------------------------------------
	// Pixel layer parameters
	//-------------------------------------------------------------------------
	static uint PIXEL_HOMOG_DELTA;
	static uint PIXEL_HOMOG_RADIUS;
	static float PIXEL_HOMOG_THRESHOLD;
	static bool PIXEL_RANDOM_INIT;

public:
	static void SetConfig(const boost::property_tree::ptree& tree);
	static bool LoadConfigFile(string file_name);
	static void GenerateConfigFile(string file_name);

	static inline string FromWorkingDir(const string& a_file)
	{
		if (Config::WORKING_DIR == "") return a_file;
		return Config::WORKING_DIR + '/' + a_file;
	}
	
};
