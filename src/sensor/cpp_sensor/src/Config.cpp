/** @file Config.cpp
 *
 *  @author Vincent de Ladurantaye
 */

#include <iostream>
#include <cstring>

#include <boost/property_tree/ini_parser.hpp>
//#include <boost/property_tree/json_parser.hpp>

#include "Config.h"


string Config::WORKING_DIR = "";

float Config::POT_THRESHOLD = 1.0f;
float Config::TAU = 1.0f;
float Config::GLOBAL_INHIB_VAL = 0.002f;
float Config::CHARGING_LEADER = 1.01f;
float Config::CHARGING_FOLLOWER = 0.5f;

float Config::SEG_WEIGHT_MAX = 0.01f;
float Config::SEG_WEIGHT_SLOPE = 1.2f;
float Config::SEG_WEIGHT_OFFSET = 0.0f;

float Config::MATCHING_WEIGHT_MAX = 1.0f;
float Config::MATCHING_WEIGHT_SLOPE = 1.0f;
float Config::MATCHING_WEIGHT_OFFSET = 10.0f;

uint Config::SEG_MAX_CASCADES = 0;
uint Config::SEG_MAX_CYCLES = 50;

bool Config::SEG_TRIGGER_SAME_LABEL_NEURONS = false;
bool Config::SEG_MERGE_SEGMENTS = false;
float Config::SEG_MERGE_DELTA = 2.0f;

uint Config::MIN_SEGMENT_SIZE = 80;

bool Config::RESIZE_IMG_KEEP_RATIO = false;
uint Config::KEEP_RATIO_LONGEST_IMG_SIDE = 150;

bool Config::FIXED_INPUT_IMGS_SIZE = false;
uint Config::FIXED_INPUT_IMGS_WIDTH = 64;
uint Config::FIXED_INPUT_IMGS_HEIGHT = 128;

uint Config::PIXEL_HOMOG_DELTA = 55;
uint Config::PIXEL_HOMOG_RADIUS = 4;
float Config::PIXEL_HOMOG_THRESHOLD = 0.6f;
bool Config::PIXEL_RANDOM_INIT = true;


//=============================================================================
//								Config File
//=============================================================================
void Config::SetConfig(const boost::property_tree::ptree& tree)
{
	//for (boost::property_tree::ptree::const_iterator it = tree.begin();
	//	it != tree.end(); ++it)
 //   {
	//	cout << '[' << it->first<< "]\n";
	//	for (boost::property_tree::ptree::const_iterator s = 
	//		 it->second.begin();
	//	s != it->second.end(); ++s)
	//	{
	//		cout << s->first << "=" 
	//			 << s->second.get_value<std::string>() << "\n";
	//	}
 //   }

	//-------------------------------------------------------------------------
	// Neuron
	//-------------------------------------------------------------------------
	POT_THRESHOLD = tree.get<float>("Neuron.POT_THRESHOLD", POT_THRESHOLD);
	TAU = tree.get<float>("Neuron.TAU", TAU);
	GLOBAL_INHIB_VAL = tree.get<float>("Neuron.GLOBAL_INHIB_VAL",
									   GLOBAL_INHIB_VAL);
	CHARGING_LEADER = tree.get<float>("Neuron.CHARGING_LEADER",
									  CHARGING_LEADER);
	CHARGING_FOLLOWER = tree.get<float>("Neuron.CHARGING_FOLLOWER",
										CHARGING_FOLLOWER);

	//-------------------------------------------------------------------------
	// Neural connexion parameters
	//-------------------------------------------------------------------------
	SEG_WEIGHT_MAX = tree.get<float>("NeuralConnexion.SEG_WEIGHT_MAX",
									 SEG_WEIGHT_MAX);
	SEG_WEIGHT_SLOPE = tree.get<float>("NeuralConnexion.SEG_WEIGHT_SLOPE",
									   SEG_WEIGHT_SLOPE);
	SEG_WEIGHT_OFFSET = tree.get<float>("NeuralConnexion.SEG_WEIGHT_OFFSET",
										SEG_WEIGHT_OFFSET);

	//-------------------------------------------------------------------------
	// General simulation parameters
	//-------------------------------------------------------------------------
	SEG_MAX_CASCADES = tree.get<uint>("SimulationParams.SEG_MAX_CASCADES",
									  SEG_MAX_CASCADES);
	SEG_MAX_CYCLES = tree.get<uint>("SimulationParams.SEG_MAX_CYCLES",
									SEG_MAX_CYCLES);

	SEG_TRIGGER_SAME_LABEL_NEURONS =
		tree.get<bool>("SimulationParams.SEG_TRIGGER_SAME_LABEL_NEURONS",
					   SEG_TRIGGER_SAME_LABEL_NEURONS);
	SEG_MERGE_SEGMENTS = tree.get<bool>("SimulationParams.SEG_MERGE_SEGMENTS",
										SEG_MERGE_SEGMENTS);
	SEG_MERGE_DELTA = tree.get<float>("SimulationParams.SEG_MERGE_DELTA",
									  SEG_MERGE_DELTA);

	MIN_SEGMENT_SIZE = tree.get<uint>("SimulationParams.MIN_SEGMENT_SIZE",
									  MIN_SEGMENT_SIZE);
	//cout << "Setup Max Cycles: " << Config::SEG_MAX_CYCLES << endl;

	//-------------------------------------------------------------------------
	// Input Image parameters
	//-------------------------------------------------------------------------
	RESIZE_IMG_KEEP_RATIO =
		tree.get<bool>("InputImageParams.RESIZE_IMG_KEEP_RATIO",
					   RESIZE_IMG_KEEP_RATIO);
	KEEP_RATIO_LONGEST_IMG_SIDE =
		tree.get<uint>("InputImageParams.KEEP_RATIO_LONGEST_IMG_SIDE",
					   KEEP_RATIO_LONGEST_IMG_SIDE);
	FIXED_INPUT_IMGS_SIZE =
		tree.get<bool>("InputImageParams.FIXED_INPUT_IMGS_SIZE",
					   FIXED_INPUT_IMGS_SIZE);
	FIXED_INPUT_IMGS_WIDTH =
		tree.get<uint>("InputImageParams.FIXED_INPUT_IMGS_WIDTH",
					   FIXED_INPUT_IMGS_WIDTH);
	FIXED_INPUT_IMGS_HEIGHT =
		tree.get<uint>("InputImageParams.FIXED_INPUT_IMGS_HEIGHT",
					   FIXED_INPUT_IMGS_HEIGHT);

	//-------------------------------------------------------------------------
	// Pixel layer parameters
	//-------------------------------------------------------------------------
	PIXEL_HOMOG_DELTA = tree.get<uint>("PixelsParams.PIXEL_HOMOG_DELTA",
									   PIXEL_HOMOG_DELTA);
	PIXEL_HOMOG_RADIUS = tree.get<uint>("PixelsParams.PIXEL_HOMOG_RADIUS",
										PIXEL_HOMOG_RADIUS);
	PIXEL_HOMOG_THRESHOLD =
		tree.get<float>("PixelsParams.PIXEL_HOMOG_THRESHOLD",
						PIXEL_HOMOG_THRESHOLD);
	PIXEL_RANDOM_INIT = tree.get<bool>("PixelsParams.PIXEL_RANDOM_INIT",
									   PIXEL_RANDOM_INIT);
}

//=============================================================================
bool Config::LoadConfigFile(string filename)
{
	boost::property_tree::ptree tree;

	cout << "\nLoading config file: " 
		<< Config::FromWorkingDir(filename) << endl;

	//try { boost::property_tree::json_parser::read_json(filename, tree); }
	try { boost::property_tree::ini_parser::read_ini(filename, tree); }
	catch (...)
	{
		Config::GenerateConfigFile(filename);
		return false;
	}

	Config::SetConfig(tree);
	return true;
}

//=============================================================================
void Config::GenerateConfigFile(string filename)
{
	boost::property_tree::ptree tree;
	
	//-------------------------------------------------------------------------
	// Neuron
	//-------------------------------------------------------------------------
	tree.put("Neuron.POT_THRESHOLD", POT_THRESHOLD);
	tree.put("Neuron.TAU", TAU);
	tree.put("Neuron.GLOBAL_INHIB_VAL", GLOBAL_INHIB_VAL);
	tree.put("Neuron.CHARGING_LEADER", CHARGING_LEADER);
	tree.put("Neuron.CHARGING_FOLLOWER", CHARGING_FOLLOWER);

	//-------------------------------------------------------------------------
	// Neural connexion parameters
	//-------------------------------------------------------------------------
	tree.put("NeuralConnexion.SEG_WEIGHT_MAX", SEG_WEIGHT_MAX);
	tree.put("NeuralConnexion.SEG_WEIGHT_SLOPE", SEG_WEIGHT_SLOPE);
	tree.put("NeuralConnexion.SEG_WEIGHT_OFFSET", SEG_WEIGHT_OFFSET);

	//-------------------------------------------------------------------------
	// General simulation parameters
	//-------------------------------------------------------------------------
	tree.put("SimulationParams.SEG_MAX_CASCADES", SEG_MAX_CASCADES);
	tree.put("SimulationParams.SEG_MAX_CYCLES", SEG_MAX_CYCLES);
	tree.put("SimulationParams.SEG_TRIGGER_SAME_LABEL_NEURONS", 
			 SEG_TRIGGER_SAME_LABEL_NEURONS);
	tree.put("SimulationParams.SEG_MERGE_SEGMENTS", SEG_MERGE_SEGMENTS);
	tree.put("SimulationParams.SEG_MERGE_DELTA", SEG_MERGE_DELTA);

	//-------------------------------------------------------------------------
	// Pixel layer parameters
	//-------------------------------------------------------------------------
	tree.put("PixelsParams.PIXEL_HOMOG_DELTA", PIXEL_HOMOG_DELTA);
	tree.put("PixelsParams.PIXEL_HOMOG_RADIUS", PIXEL_HOMOG_RADIUS);
	tree.put("PixelsParams.PIXEL_HOMOG_THRESHOLD", PIXEL_HOMOG_THRESHOLD);
	tree.put("PixelsParams.PIXEL_RANDOM_INIT", PIXEL_RANDOM_INIT);
	

	//try { boost::property_tree::json_parser::write_json(filename, tree); }
	try { boost::property_tree::ini_parser::write_ini(
		Config::FromWorkingDir(filename), tree); }
	catch (...)	{}

}
