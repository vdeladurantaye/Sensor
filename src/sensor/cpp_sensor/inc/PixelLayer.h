/** @file PixelLayer.h
 *
 *  @author Vincent de Ladurantaye
 */
#pragma once

#include "SegmentationLayer.h"

 /**
 * Layer that does segmentation based on gray pixel values of images
 */
class PixelLayer : public SegmentationLayer
{
public:

	/**
	* Constructor
	*/
	PixelLayer(const string& a_img_file,
			   bool a_random_init = Config::PIXEL_RANDOM_INIT);
	PixelLayer(const cv::Mat& a_img,
			   bool a_random_init = Config::PIXEL_RANDOM_INIT);
	PixelLayer(ImageData& a_img_data,
			   bool a_random_init = Config::PIXEL_RANDOM_INIT);

public:

	// Pointer to image gray pixel values
	const uchar* pixel_data;

protected:
	
	/**
	* Calculates the weights between two neurons in the layer
	*/
	float ComputeWeigth(int a_src_id, int a_dst_id,
						NeuronRelPos a_dst_pos);

	/**
	* Calculates the homogeneity of pixel values in an area. Neurons in
	* homogeneous areas will be leaders.
	*/
	double GetHomogeneity(int a_x, int a_y, int a_radius);


//-----------------------------------------------------------------------------
//							Configuration Parameters
//-----------------------------------------------------------------------------
public:
	// Homogeneity tolerated pixel delta
	int HOMOG_DELTA;
	// Homogeneity area radius
	int HOMOG_RADIUS;
	// Homogeneity leader threshold
	float HOMOG_THRESHOLD;

	// Flag determining if inital neural potential is random or proportional
	// to pixel value
	const bool RANDOM_INIT;

};
