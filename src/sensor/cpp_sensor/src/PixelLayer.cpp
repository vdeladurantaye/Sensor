/** @file PixelLayer.cpp
 *
 *
 *  @author Vincent de Ladurantaye
 */

#include "PixelLayer.h"



//=============================================================================
//								PixelLayer
//=============================================================================
PixelLayer::PixelLayer(const string& a_img_file, bool a_random_init):
PixelLayer(ImageData(a_img_file), a_random_init)
{
}

//-----------------------------------------------------------------------------
PixelLayer::PixelLayer(const cv::Mat& a_img, bool a_random_init):
	PixelLayer(ImageData(a_img), a_random_init)
{
}

//-----------------------------------------------------------------------------
PixelLayer::PixelLayer(ImageData& a_img_data, bool a_random_init):
	SegmentationLayer(a_img_data),
	HOMOG_DELTA(Config::PIXEL_HOMOG_DELTA),
	HOMOG_RADIUS(Config::PIXEL_HOMOG_RADIUS),
	HOMOG_THRESHOLD(Config::PIXEL_HOMOG_THRESHOLD),
	RANDOM_INIT(a_random_init)
{
	// Keep a ptr to the image pixel data
	pixel_data = img_data_.gray_image_.data;

	for (auto& n : neurons)
	{
		if (GetHomogeneity(n.pos.x, n.pos.y, HOMOG_RADIUS) > HOMOG_THRESHOLD)
		{
			n.max_charge = CHARGING_LEADER;
		}
		else
		{
			n.max_charge = CHARGING_FOLLOW;
		}

		if (RANDOM_INIT)
		{
			n.pot = ((float)rand() / RAND_MAX) * POT_THRESHOLD;
		}
		else
		{
			n.pot = 0.99 * POT_THRESHOLD * (pixel_data[n.id] / 255.0f);
		}
	}
}

//=============================================================================
float PixelLayer::ComputeWeigth(int a_src_id, int a_dst_id,
								NeuronRelPos a_dst_pos)
{
	int featDiff = abs(pixel_data[a_src_id] - pixel_data[a_dst_id]);
	
	return SegmentationLayer::ComputeWeigth(featDiff);
}

//=============================================================================
double PixelLayer::GetHomogeneity(int a_x, int a_y, int a_radius)
{
	double similarNeighbor; // How many surrounding neurons are similar
	double totalNeighbor;   // How many surrounding neurons are in image

	similarNeighbor = 0.0;
	totalNeighbor = 0.0;

	const uchar* dataPtr = pixel_data + a_y*width;

	for (int dy = -a_radius; dy <= a_radius; dy++)
	{
		const uchar* deltaPtr = pixel_data + (a_y + dy)*width;

		for (int dx = -a_radius; dx <= a_radius; dx++)
		{
			if (IsInLayer(a_x + dx, a_y + dy) && ((dy * dy) + (dx * dx)))
			{
				if (fabs(dataPtr[a_x] - deltaPtr[a_x + dx]) < HOMOG_DELTA)
				{
					similarNeighbor += 1.0;
				}
				totalNeighbor += 1.0;
			}
		}
	}
	return (similarNeighbor / totalNeighbor);
}

