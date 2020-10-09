/** @file Tools.h
 * This file contains different utility functions and structures as well
 * as generic include lines.
 *
 *  @author Vincent de Ladurantaye
 */
#pragma once

#include <vector>
#include <list>
#include <memory>
using namespace std;

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
using cv::Mat;
//using namespace cv;

#include "Config.h"



//=============================================================================
//								Generic Functions
//=============================================================================
/**
* Display an image. Used to display images with standardised config.
*/
static void DisplayImage(const string& a_title, const Mat& a_image)
{
	//namedWindow(title, CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO);
	cv::namedWindow(a_title, CV_GUI_EXPANDED | CV_WINDOW_KEEPRATIO);

	// If a window of the same name was previously used, resize the 
	// window to this new image's dimensions
	cv::resizeWindow(a_title, a_image.cols, a_image.rows);

	// Display the image
	cv::imshow(a_title, a_image);

	// Allow the program to display right away
	cv::waitKey(1);
}

//inline string FromWorkingDir(const string& a_file)
//{
//	if (Config::WORKING_DIR == "") return a_file;
//	return Config::WORKING_DIR + '/' + a_file;
//}
