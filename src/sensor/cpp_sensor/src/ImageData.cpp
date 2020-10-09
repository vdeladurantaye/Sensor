/**
 * @file ImageData.cpp
 *
 * @author Vincent de Ladurantaye
 */

#include "ImageData.h"
#include "Monitor.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm> 

using namespace std;


// Compute PI
const float PI = atan(1.0f) * 4.0f;


//=============================================================================
ImageData::ImageData():
	rows(0),
	cols(0),
	size(0)
{
}

//=============================================================================
ImageData::ImageData(const string& imageName) :
	rows(0),
	cols(0),
	size(0)
{
	//cout << "\nLoading file: " << imageName << endl;
	SetImage(Config::FromWorkingDir(imageName));
}

//=============================================================================
ImageData::ImageData(const Mat& image) :
	rows(0),
	cols(0),
	size(0)
{
	SetImage(image);
}

//=============================================================================
void ImageData::SetImage(const Mat& image)
{
	// Check if image loaded correctly
	if (image.empty())
	{
		cout << "Received an empty image!" << endl;
		exit(-1);
	}

	// Todo: ManageFormat always converts to grayscale, which we might not always want
	ManageFormat(image);


	// Save the size of the image in member variables for cleaner access
	rows = image_.rows;
	cols = image_.cols;
	size = rows*cols;
}

//=============================================================================
void ImageData::SetImage(int rows, int cols, uchar* data, int chan)
{
	Mat image;

	switch (chan)
	{
	default:
	case 1:
		// Create an image with a single channel
		image = Mat(rows, cols, CV_8UC1, data);
		break;

	case 3:
		// Create a 3 channel color image
		image = Mat(rows, cols, CV_8UC3, data);
		break;

	case 4:
		// Create a 3 channel color image with an alpha channel
		image = Mat(rows, cols, CV_8UC4, data);
		break;
	}

	SetImage(image);
}

//=============================================================================
void ImageData::SetImage(string imageName)
{
	Mat image = cv::imread(imageName,-1);

	if (image.empty())
	{
		cout << "Cannot read image " << imageName << "!" << endl;
		exit(-1);
	}

	img_filename_ = imageName;
	
	SetImage(image);
}

//=============================================================================
void ImageData::SetImage(int rows, int cols, float* float_data)
{
	vector<unsigned char> uchar_data(rows*cols);

	for (int i=0; i<rows*cols; ++i)
	{
		uchar_data[i] = float_data[i];
	}
	Mat image(rows, cols, CV_8UC1, uchar_data.data());

	SetImage(image);
}

//=============================================================================
void ImageData::SetImage(int p_row, int p_col, int* p_data)
{
	vector<unsigned char> uchar_data(p_row*p_col);

	for (int i=0; i<p_row*p_col; ++i)
	{
		uchar_data[i] = p_data[i];
	}
	Mat image(p_row, p_col, CV_8UC1, uchar_data.data());

	SetImage(image);
}

//=============================================================================
void ImageData::SetVideoSource(string p_imageName)
{
	video_.open(p_imageName);

	if(!video_.isOpened())
	{
		cout << "Error opening video" << endl;
		exit(-1);
	}

	int time = 0;
	video_.set(CV_CAP_PROP_POS_MSEC, time);

	Mat frame;
	video_ >> frame;

	if (frame.empty())
	{
		cout << "Error reading video" << endl;
		exit(-1);
	}

	img_filename_ = p_imageName;

	SetImage(frame);
}

//=============================================================================
void ImageData::ManageFormat(const Mat& image)
{
	cv::Size size(image.cols, image.rows);

	// Resize images keeping their original ratio
	if (Config::RESIZE_IMG_KEEP_RATIO)
	{
		float ratio;
		if (image.rows > image.cols) 
			ratio = (float) Config::KEEP_RATIO_LONGEST_IMG_SIDE / image.rows;
		else ratio = (float) Config::KEEP_RATIO_LONGEST_IMG_SIDE / image.cols;
		
		if (ratio < 1)
		{
			size.height = image.rows * ratio;
			size.width = image.cols * ratio;
		}

		cv::resize(image, image_, size);
	}
	else image_ = image;

	// If the image has to have a very specific size
	if (Config::FIXED_INPUT_IMGS_SIZE)
	{
		// Crop the image from the center
		cv::Rect roi;

		roi.height = Config::FIXED_INPUT_IMGS_HEIGHT;
		roi.width = Config::FIXED_INPUT_IMGS_WIDTH;

		roi.x = (image_.cols - roi.width) / 2 - 1;
		roi.y = (image_.rows - roi.height) / 2 - 1;

		// If image is smaller than the desired size, resize it
		if (roi.x < 0 || roi.y < 0)
		{
			size.height = Config::FIXED_INPUT_IMGS_HEIGHT;
			size.width = Config::FIXED_INPUT_IMGS_WIDTH;
			cv::resize(image_, image_, size);
		}
		// Else, crop it
		else if (image_.type() == CV_8UC3)
		{
			//BUG: Croping the image using the normal OpenCV method sometimes 
			// generates a random SEH exception, so we create a new image and 
			// copy the data.
			// This might be due to the feature computations that are using the
			// image data outside of the cropping box
			image_ = Mat(roi.height, roi.width, image.type());

			for (int r = 0; r < roi.height; ++r)
			{
				const cv::Vec3b* orig = image.ptr<cv::Vec3b>(r + roi.y);
				cv::Vec3b* roiCopy = image_.ptr<cv::Vec3b>(r);
				for (int c = 0; c < roi.width; ++c)
				{
					roiCopy[c] = orig[c + roi.x];
				}
			}
		}
		else image_ = image(roi);
	}

	//// Crop the image if needed
	//if (crop_roi.width > 0 && crop_roi.height > 0)
	//{
	//	cv::Mat(image, crop_roi).copyTo(image_);
	//}

	// Get the grayscale version of the image
	switch (image_.type())
	{
	case CV_8UC1:
		// If already grayscale, simply refer to the same image
		gray_image_ = image_;
		cv::cvtColor(image_, image_, cv::COLOR_GRAY2BGR);
		break;

	case CV_8UC3:
		//for (int i = 0; i < image_.rows; ++i)
		//for (int j = 0; j < image_.cols; ++j)
		//{
		//	Vec3b& rgba = image_.at<Vec3b>(i, j);
		//	rgba[0] = 0; // Blue
		////	green_img.at<uchar>(i, j) = rgba[1];
		////	red_img.at<uchar>(i, j) = rgba[2];
		//}
		// If RGB, convert to grayscale
		cv::cvtColor(image_, gray_image_, CV_BGR2GRAY);
		break;

	case CV_8UC4:
		// If RGBA, convert to grayscale
		cv::cvtColor(image_, gray_image_, CV_BGRA2GRAY);

		// Get the alpha matrice
		alpha_ = Mat(image_.rows, image_.cols, CV_8UC1);
		for (int i = 0; i < image_.rows; ++i)
		for (int j = 0; j < image_.cols; ++j)
		{
			cv::Vec4b& rgba = image_.at<cv::Vec4b>(i, j);
			alpha_.at<uchar>(i, j) = rgba[3];
		}

		// Convert to BGR
		// TODO: Test this, this has been changed but not tested
		cv::cvtColor(image_, image_, CV_BGRA2BGR);
		break;

	case CV_32F:// Float data
		{
			Mat displayImg = Mat(image_.rows, image_.cols, CV_8UC1);
			uchar* dipsPtr = displayImg.ptr();
			const float* dataPtr = image.ptr<float>();
			for (int p = 0; p < image_.total(); ++p)
			{
				dipsPtr[p] = std::min(255, int(dataPtr[p] * 255/5));
			}
			image_ = displayImg;
		}
		break;

	default:
		cout << "Can not manage image format!" << endl;
		exit(-1);
		break;
	}

}
