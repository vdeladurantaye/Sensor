/** @file test_odlm.cpp
*
* @authors Vincent de Ladurantaye
*/
#include "test_pixel.h"
#include "LayerCoupler.h"

#include "LayerDebugger.h"
#include "Monitor.h"

#include <chrono>
#include <fstream>
using namespace::std;


const int DISPLAY_TIME = 1000;

//=============================================================================
//								  TestOdlmPixel
//=============================================================================
TestOdlmPixel::TestOdlmPixel()
{
}

//=============================================================================
void TestOdlmPixel::SetUp()
{
	Config::LoadConfigFile("SensorPixel.ini");
}

//=============================================================================
void TestOdlmPixel::TearDown()
{

}

//=============================================================================
//									TESTS
//=============================================================================
TEST_F(TestOdlmPixel, DISABLED_Segmentation)
{
	bool randomInit = true;
	bool regenerateValidationFile = false;
	
	string imgFilename = "carGray.bmp";
	ImageData imgData(imgFilename);

	PixelLayer pixelSegLayer(imgData, randomInit);
	
	SegmentLayerMonitor label_monitor("Segment Monitor",
									  &pixelSegLayer,
									  imgData, DSM_MONITOR_SEGMENTS);

	using namespace std::chrono;
	high_resolution_clock::time_point tStart, tEnd;
	tStart = high_resolution_clock::now();

	pixelSegLayer.SegmentLayer();

	tEnd = std::chrono::high_resolution_clock::now();
	duration<double> time_span =
		duration_cast<duration<double>>(tEnd - tStart);
	cout << "Segmentation time: " << time_span.count() << " seconds" << endl;

	string validFilename = "test/TEST_Segmentation_" + 
		imgFilename.substr(0, imgFilename.find('.')) +
		(randomInit ? "_randInit": "") +
		".valid";

	if (regenerateValidationFile)
		pixelSegLayer.SaveStateToFile(validFilename);
	else
		EXPECT_TRUE(pixelSegLayer.ValidateLayerState(validFilename));

	label_monitor.Display();

	cv::waitKey(DISPLAY_TIME);
}

//=============================================================================
TEST_F(TestOdlmPixel, DISABLED_SegmentationDebugger)
{
	ImageData imgData("carGray.bmp");

	PixelLayer pixelSegLayer(imgData);

	//pixelSegLayer.TRIGGER_SAME_LABEL_NEURONS = true;
	//pixelSegLayer.MERGE_SEGMENTS = true;
	
	SegmentLayerMonitor potentials_monitor("Potentials Monitor",
										   &pixelSegLayer,
										   imgData, DSM_MONITOR_POTENTIAL);
	SegmentLayerMonitor label_monitor("Segment Monitor",
									  &pixelSegLayer,
									  imgData, DSM_MONITOR_SEGMENTS);


	LayerDebugger::AddDebugger(pixelSegLayer, "Pixels");
	LayerDebugger::SetDebugLvl(DEBUG_LEVEL_CASCADE);
	
	pixelSegLayer.SegmentLayer();

	//cv::waitKey(DISPLAY_TIME);
	cv::waitKey(0);
}

//=============================================================================
TEST_F(TestOdlmPixel, Misc_Test)
{
	bool randomInit = true;
	bool regenerateValidationFile = false;

	string imgFilename = "small.bmp";
	ImageData imgData(imgFilename);

	PixelLayer pixelSegLayer(ImageData(imgFilename), randomInit);

	//{
		SegmentLayerMonitor label_monitor("Segment Monitor", pixelSegLayer);
		label_monitor.Display();
		LayerDebugger::AddDebugger(pixelSegLayer, "Debug");
	//}


	pixelSegLayer.SegmentLayer();
	

	cv::waitKey(DISPLAY_TIME);
}