/**
 * @file ImageData.h
 *
 * @authors Vincent de Ladurantaye
 */
#pragma once

 #include "Tools.h"

//#include <opencv2/video/tracking.hpp>
//#include "opencv2/calib3d/calib3d.hpp"
//using namespace cv;

//#include "FeatExtractor.h"
//#include "HogExtractor.h"


class ImageData
{
public:

	/**
	* Constructor
	*/
	ImageData();
	ImageData(const string& imageName);
	ImageData(const Mat& image);

	/**
	* Destructor
	*/
	~ImageData(){}


	/**
	* Set the image data
	*/
	void SetImage(const Mat& image);
	void SetImage(
		int rows,
		int cols,
		uchar* data,
		int chan);
	void SetImage(string imageName);
	void SetImage(
		int p_row,
		int p_col,
		float* p_data);
	void SetImage(
		int p_row,
		int p_col,
		int* p_data);

	
	void SetVideoSource(string p_imageName);

	const Mat& GetImage() const {return image_;} 

public:
	// Image height
	size_t rows;
	// Image width
	size_t cols;
	// Image size
	size_t size;
	
	cv::VideoCapture video_;
	

	// Get the image file name
	const string& GetFilename() const {return img_filename_;}

	// Original image, may be up to 4 channels
	Mat image_;
	// Original grayscale version of the image used, among other things for
	// high level feature extraction
	Mat gray_image_;

private:

	// Manage different image formats and sets the image members
	void ManageFormat(const Mat& image);


	// Image filename if available
	string img_filename_;
	

	Mat alpha_;
    
	vector<cv::Vec3b> ignored_colors;

};
