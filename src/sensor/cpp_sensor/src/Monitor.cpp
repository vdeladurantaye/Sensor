/** @file Monitor.cpp
 *
 *  @author Vincent de Ladurantaye
 */

#ifdef WIN32
#include <Windows.h>
#endif

#include "Monitor.h"

#include <iostream>
using namespace std;

//=============================================================================
//									Monitor
//=============================================================================
// List of all existing monitors
list<Monitor*> Monitor::monitors_;

//=============================================================================
Monitor::Monitor(const string& a_name)
	: title_(a_name),
	  disp_width_(0),
	  disp_height_(0),
	  min_width_(0),
	  min_height_(0),
	  disp_zoom_(1, 1),
	  is_child_(false)
{
	// Add the monitor to the monitor list
	monitors_.push_back(this);
}

//=============================================================================
Monitor::~Monitor()
{
	// Find the monitor in the list and erase it
	list<Monitor*>::iterator m =
		find(monitors_.begin(), monitors_.end(), this);
	if (m != monitors_.end())
		monitors_.erase(m);
}

//=============================================================================
void Monitor::Display()
{
	// Create the window
	cv::namedWindow(title_, CV_WINDOW_NORMAL | // CV_WINDOW_AUTOSIZE
						CV_WINDOW_FREERATIO | // CV_WINDOW_KEEPRATIO
						CV_GUI_EXPANDED); // CV_GUI_NORMAL 

	cvSetMouseCallback(title_.c_str(), &MouseCallback, this);

	// If on windows, get the window size and calculate the display zoom
	// Not sure how to the window management works on MacOS. The better
	// solution would be to transfer the monitor class to Python.
#ifdef WIN32
	HWND hWnd = static_cast<HWND>(cvGetWindowHandle(title_.c_str()));

	RECT lpRect;
	GetWindowRect(hWnd, &lpRect);

	//SetWindowPos(hWnd, HWND_TOP, 0, 0, disp_width_, disp_height_,
	//			 SWP_NOOWNERZORDER|SWP_NOREPOSITION|SWP_SHOWWINDOW);

	disp_width_ = lpRect.right - lpRect.left;
	disp_height_ = lpRect.bottom - lpRect.top;
#endif

	// Check for min size
	if (disp_width_ < min_width_)
		disp_width_ = min_width_;
	if (disp_height_ < min_height_)
		disp_height_ = min_height_;

	// Recreate the display if display size has changed
	display_.create(disp_height_, disp_width_, display_.type());

	// Calculate the window resize zoom
	disp_zoom_.x = (float)disp_width_ / min_width_;
	disp_zoom_.y = (float)disp_height_ / min_height_;

	// Draw the image
	Draw();

	// Display the image
	imshow(title_, display_);

	// Call cv::waitKey with 1ms so the image is displayed right away
	cv::waitKey(1);
}

//=============================================================================
cv::Mat Monitor::GetDisplay()
{
	Draw();
	return display_.clone();
}

//=============================================================================
void Monitor::ResetSize()
{
	disp_width_ = min_width_;
	disp_height_ = min_height_;
	cv::resizeWindow(title_, disp_width_, disp_height_);
}

//=============================================================================
void Monitor::RefreshMonitors()
{
	for (auto& monitor : monitors_)
	{
		monitor->Display();
	}
}

//=============================================================================
//								Image Monitor
//=============================================================================
ImageMonitor::ImageMonitor(const string& a_name, const Mat &a_image)
	: Monitor(a_name),
	  image_(a_image.clone()),
	  roi_(0, 0, a_image.cols, a_image.rows),
	  drag_start_(0, 0),
	  scroll_zoom_(1),
	  zoom_(1, 1),
	  cell_color_(cv::Scalar(0, 255, 0))
{
	display_.create(a_image.rows, a_image.cols, a_image.type());

	min_width_ = a_image.cols;
	min_height_ = a_image.rows;

	disp_width_ = min_width_;
	disp_height_ = min_height_;
}

//=============================================================================
ImageMonitor::~ImageMonitor()
{
}

//=============================================================================
void ImageMonitor::Draw()
{
	// Calculate the zoom
	zoom_.x = scroll_zoom_ * disp_zoom_.x;
	zoom_.y = scroll_zoom_ * disp_zoom_.y;

	DrawRoi();

	DrawCells(cell_color_);
}

//=============================================================================
void ImageMonitor::SaveAsImage(string a_filename)
{
	// Create the display with the size of the original image
	display_.create(image_.rows, image_.cols, display_.type());
	disp_width_ = image_.cols;
	disp_height_ = image_.rows;

	// Draw the image
	Draw();

	imwrite(a_filename, display_);
}

//=============================================================================
void ImageMonitor::MouseHandler(int a_event, int a_x, int a_y, int a_flags)
{
	// Absolute cursor position, that is, the position in the image,
	// regardless of the zoom and translation
	cv::Point absPos;
	absPos.x = roi_.x + a_x / zoom_.x;
	absPos.y = roi_.y + a_y / zoom_.y;

	switch (a_event)
	{
	case CV_EVENT_LBUTTONDOWN:
	case CV_EVENT_RBUTTONDOWN:
		drag_start_.x = a_x;
		drag_start_.y = a_y;
		return;

	case CV_EVENT_LBUTTONUP:
	case CV_EVENT_RBUTTONUP:
		// If dragging
		if (drag_start_.x != a_x && drag_start_.y != a_y)
		{
			// On button up, calculate the distance dragged
			roi_.x = roi_.x +
					 ((float)drag_start_.x - a_x) / zoom_.x;
			roi_.y = roi_.y +
					 ((float)drag_start_.y - a_y) / zoom_.y;
		}
		else
		{
			OnMouseClick(a_event, absPos.x, absPos.y, a_flags);
			return;
		}

		break;

	case CV_EVENT_MOUSEMOVE:
#ifdef HAVE_QT
		char overlay[50];
		sprintf(overlay, "x:%i y:%i", 50, a_x, a_y);
		displayOverlay(title_, overlay);
#endif
		OnMouseMove(absPos.x, absPos.y, a_flags);
		//cout << "Mouse: x:" << a_x << " y:" << a_y
		//	<< " zoom:" << scroll_zoom_ << endl;
		return;

	case CV_EVENT_MOUSEWHEEL:
	case CV_EVENT_MOUSEHWHEEL:

		// Position of the cursor relative to the window size
		float relX = a_x / (float)disp_width_;
		float relY = a_y / (float)disp_height_;

		// Check if zooming in or out
		int delta = cv::getMouseWheelDelta(a_flags);
		if (delta > 0)
		{
			scroll_zoom_ += 2;
			if (scroll_zoom_ > 100)
				scroll_zoom_ = 100;
		}
		else
		{
			scroll_zoom_ -= 2;
			if (scroll_zoom_ < 1)
				scroll_zoom_ = 1;
		}

		cout << "Zoom:" << scroll_zoom_ << " x" << endl;

		// ROI size
		roi_.height = image_.rows / scroll_zoom_;
		roi_.width = image_.cols / scroll_zoom_;
		// ROI position
		roi_.x = absPos.x - relX * roi_.width + 1;
		roi_.y = absPos.y - relY * roi_.height + 1;

		break;
	}

	// Check boundaries of the region of interest
	if (roi_.x + roi_.width > image_.cols)
	{
		roi_.x = image_.cols - roi_.width;
	}
	else if (roi_.x < 0)
	{
		roi_.x = 0;
	}
	if (roi_.y + roi_.height > image_.rows)
	{
		roi_.y = image_.rows - roi_.height;
	}
	else if (roi_.y < 0)
	{
		roi_.y = 0;
	}

	// Redraw
	if (!is_child_)
		Display();
}

//=============================================================================
void ImageMonitor::OnMouseClick(int a_event, int a_x, int a_y, int a_flags)
{
	switch (image_.type())
	{
	case CV_8UC1:
		cout << "x:" << a_x << " y:" << a_y
			 << " Pixel:" << (int)image_.at<uchar>(a_y, a_x) << endl;
		break;

	case CV_8UC3:
		cout << "x:" << a_x << " y:" << a_y
			 << " Pixel:" << image_.at<cv::Vec3b>(a_y, a_x) << endl;
		break;
	case CV_32F:
		cout << "x:" << a_x << " y:" << a_y
			 << " Pixel:" << image_.at<float>(a_y, a_x) << endl;
		break;
	}
}

//=============================================================================
void ImageMonitor::OnMouseMove(int a_x, int a_y, int a_flags)
{
}

//=============================================================================
void ImageMonitor::DrawRoi()
{
	// Draw depending on image type
	switch (image_.type())
	{
	case CV_8UC1: // Grayscale image
		// Recreate the display to ensure grayscale
		display_.create(disp_height_, disp_width_, CV_8UC1);

		for (uint y = 0; y < disp_height_; ++y)
		{
			char *disp = display_.ptr<char>(y);
			char *img = image_.ptr<char>(
				floor(y / zoom_.y) + roi_.y);
			for (uint x = 0; x < disp_width_; ++x)
			{
				int absX = floor(x / zoom_.x) + roi_.x;

				disp[x] = img[absX];
			}
		}
		break;

	case CV_8UC3: // Color image
		// Recreate the display to ensure 3 color channels
		if (display_.type() != CV_8UC3)
			display_.create(disp_height_, disp_width_, CV_8UC3);
		for (uint y = 0; y < disp_height_; ++y)
		{
			cv::Vec3b *disp = display_.ptr<cv::Vec3b>(y);
			const cv::Vec3b *img = image_.ptr<cv::Vec3b>(
				floor(y / zoom_.y) + roi_.y);
			for (uint x = 0; x < disp_width_; ++x)
			{
				int absX = floor(x / zoom_.x) + roi_.x;

				disp[x] = img[absX];
			}
		}
		break;

	case CV_32F: // Floating point value image
		// Recreate the display to ensure 3 color channels
		if (display_.type() != CV_8UC3)
			display_.create(disp_height_, disp_width_, CV_8UC3);
		for (uint y = 0; y < disp_height_; ++y)
		{
			float *disp = display_.ptr<float>(y);
			const float *img = image_.ptr<float>(
				floor(y / zoom_.y) + roi_.y);
			for (uint x = 0; x < disp_width_; ++x)
			{
				int absX = floor(x / zoom_.x) + roi_.x;

				disp[x] = img[absX];
			}
		}
		break;
	}
}

//=============================================================================
void ImageMonitor::DrawCells(cv::Scalar &a_color)
{
	if (scroll_zoom_ < 7)
		return;

	cv::Point pt1, pt2;
	// Draw vertical lines
	pt1.y = 0;
	pt2.y = disp_height_ - 1;
	for (int c = 0; c < roi_.width + 1; ++c)
	{
		pt1.x = c * zoom_.x;
		pt2.x = c * zoom_.x;
		line(display_, pt1, pt2, a_color, 1);
	}

	// Horizontal lines
	pt1.x = 0;
	pt2.x = disp_width_ - 1;
	for (int r = 0; r < roi_.height + 1; ++r)
	{
		pt1.y = r * zoom_.y;
		pt2.y = r * zoom_.y;
		line(display_, pt1, pt2, a_color, 1);
	}
}

//=============================================================================
//								Layer Monitor
//=============================================================================
LayerMonitor::LayerMonitor(const string& a_name, 
						   const NeuralLayer& a_layer,
						   MonitorMode a_mode)
	: LayerMonitor(a_name, &a_layer, a_layer.img_data_, a_mode)
{
}
//-----------------------------------------------------------------------------
LayerMonitor::LayerMonitor(const string& a_name,
						   const NeuralLayer *a_layer,
						   const ImageData &a_image_data,
						   MonitorMode a_mode)
	: ImageMonitor(a_name, a_image_data.image_),
	  image_data_(a_image_data),
	  layer_(a_layer),
	  mode_(a_mode)
{
	mode_sequence_.push_back(DSM_MONITOR_PIXELS);
	mode_sequence_.push_back(DSM_MONITOR_POTENTIAL);
	mode_sequence_.push_back(DSM_MONITOR_CUSTOM);

	display_.create(a_image_data.rows, a_image_data.cols, CV_8UC3);
}

//=============================================================================
LayerMonitor::~LayerMonitor()
{
}

//=============================================================================
void LayerMonitor::SelectNextDisplayMode(bool a_select_prev)
{
	for (int i = 0; i < mode_sequence_.size(); ++i)
	{
		// Find the current mode
		if (mode_ == mode_sequence_[i])
		{
			// Select the previous mode
			if (a_select_prev)
			{
				// If at the start of the list, go back to last item
				if (i == 0)
					i = mode_sequence_.size();
				// Select next mode
				mode_ = mode_sequence_[i - 1];
			}
			// Select the next mode
			else
			{
				// If at the end of the list, go back to first item
				if (i == mode_sequence_.size() - 1)
					i = -1;
				// Select next mode
				mode_ = mode_sequence_[i + 1];
			}

			cout << "Displaying " << GetDisplayModeName(mode_) << endl;

			return;
		}
	}

	// If we didn't find the current mode in the list, it means the current
	// display mode is invalid, reset it to the first item in the list
	mode_ = mode_sequence_.front();
}

//=============================================================================
void LayerMonitor::SetDisplayMode(MonitorMode a_mode)
{
	mode_ = a_mode;
}

//=============================================================================
string LayerMonitor::GetDisplayModeName(MonitorMode a_mode)
{
	string name;

	switch (a_mode)
	{
	case DSM_MONITOR_PIXELS:
		name = "Pixels";
		break;

	case DSM_MONITOR_POTENTIAL:
		name = "Potentials";
		break;

	case DSM_MONITOR_ORIENTATION:
		name = "Orientations";
		break;

	case DSM_MONITOR_SEGMENTS:
		name = "Segments";
		break;

	case DSM_MONITOR_CUSTOM:
		name = "Custom values";
		break;
	}

	return name;
}

//=============================================================================
void LayerMonitor::DrawRoi()
{
	switch (mode_)
	{
	case DSM_MONITOR_CUSTOM:
		DrawCustom();
		break;

	case DSM_MONITOR_PIXELS:
		ImageMonitor::DrawRoi();
		break;

	case DSM_MONITOR_POTENTIAL:
	default:
		DrawPotential();
		break;
	}
}

//=============================================================================
void LayerMonitor::OnMouseClick(int a_event, int a_x, int a_y, int a_flags)
{
	// If CTRL is pressed, change mode
	if (a_flags & CV_EVENT_FLAG_CTRLKEY)
	{
		if (a_event == CV_EVENT_LBUTTONUP)
			SelectNextDisplayMode(true);
		if (a_event == CV_EVENT_RBUTTONUP)
			SelectNextDisplayMode(false);
		if (!is_child_)
			Display();
		return;
	}

	if (mode_ == DSM_MONITOR_PIXELS)
	{
		ImageMonitor::OnMouseClick(a_event, a_x, a_y, a_flags);
		return;
	}

	// Neuron index
	int i = a_y * layer_->width + a_x;
	cout << "index:" << i
		 << " x:" << a_x << " y:" << a_y;

	switch (mode_)
	{
	case DSM_MONITOR_CUSTOM:
		//cout << " Phase:" << layer_->neurons[i].phase << endl;
		cout << " Label:" << layer_->neurons[i].label << endl;
		break;

	case DSM_MONITOR_POTENTIAL:
	default:
		cout << " Potential:" << layer_->neurons[i].pot << endl;
		break;
	}
}

//=============================================================================
cv::Vec3b LayerMonitor::GetDisplayColor(int a_val)
{
	cv::Vec3b color;

	color.val[0] = ((a_val + 299) * 37) % 256;
	color.val[1] = ((a_val + 199) * 27) % 256;
	color.val[2] = (a_val * 13) % 256;

	return color;

}

//=============================================================================
void LayerMonitor::DrawCustom()
{
	// Ensure the display is 3 color channels
	if (display_.type() != CV_8UC3)
		display_.create(disp_height_, disp_width_, CV_8UC3);

	for (size_t y = 0; y < disp_height_; ++y)
	{
		cv::Vec3b *disp = display_.ptr<cv::Vec3b>(y);

		for (size_t x = 0; x < disp_width_; ++x)
		{
			size_t index = (floor(y / zoom_.y) + roi_.y) * layer_->width +
						   floor(x / zoom_.x) + roi_.x;

			int val = layer_->neurons[index].phase * 153;
			//int val = layer_->neurons[index].label;
			//int val = (float)layer_->neurons[index].pot *100;

			if (layer_->neurons[index].phase >= 0)
			{
				disp[x] = GetDisplayColor(val);
			}
			else
			{
				disp[x].val[0] = 0;
				disp[x].val[1] = 0;
				disp[x].val[2] = 0;
			}
		}
	}
}

//=============================================================================
void LayerMonitor::DrawPotential()
{
	Mat img(disp_height_, disp_width_, CV_32F);

	int index;
	for (size_t y = 0; y < disp_height_; ++y)
	{
		float *val = img.ptr<float>(y);
		for (size_t x = 0; x < disp_width_; ++x)
		{
			index = (floor(y / zoom_.y) + roi_.y) * layer_->width + 
				floor(x / zoom_.x) + roi_.x;

			// Use a log scale because potential rise rapidly and bunch up near
			// the threshold
			val[x] = -(log10(layer_->POT_THRESHOLD - 
							 layer_->neurons[index].pot) - 1) * 128;
		}
	}

	img.convertTo(img, CV_8U);

	// Apply the colormap:
	cv::applyColorMap(img, display_, cv::COLORMAP_JET);
}

//=============================================================================
//							  SegmentLayerMonitor
//=============================================================================
SegmentLayerMonitor::SegmentLayerMonitor(const string& a_name,
										 const SegmentationLayer *a_layer,
										 ImageData &a_image_data,
										 MonitorMode a_mode)
	: LayerMonitor(a_name, a_layer, a_image_data, a_mode)
{
	mode_sequence_.push_back(DSM_MONITOR_SEGMENTS);
}
//-----------------------------------------------------------------------------
SegmentLayerMonitor::SegmentLayerMonitor(const string& a_name,
										 const SegmentationLayer& a_layer,
										 MonitorMode a_mode)
	: LayerMonitor(a_name, a_layer, a_mode)
{
	mode_sequence_.push_back(DSM_MONITOR_SEGMENTS);
}

//=============================================================================
SegmentLayerMonitor::~SegmentLayerMonitor()
{
}

//=============================================================================
void SegmentLayerMonitor::DrawRoi()
{
	switch (mode_)
	{
	case DSM_MONITOR_SEGMENTS:
		DrawSegments();
		break;

	default:
		LayerMonitor::DrawRoi();
		break;
	}
}

//=============================================================================
void SegmentLayerMonitor::OnMouseClick(int a_event, int a_x, int a_y,
									   int a_flags)
{
	if (mode_ != DSM_MONITOR_SEGMENTS || a_flags & CV_EVENT_FLAG_CTRLKEY)
	{
		LayerMonitor::OnMouseClick(a_event, a_x, a_y, a_flags);
		return;
	}

	// Neuron index
	int i = a_y * layer_->width + a_x;

	cout << "index:" << i
		 << " x:" << a_x << " y:" << a_y
		 << " Label: " << layer_->neurons[i].label << endl;
}

//=============================================================================
void SegmentLayerMonitor::DrawSegments()
{
	// Ensure the display is 3 color channels
	if (display_.type() != CV_8UC3)
		display_.create(disp_height_, disp_width_, CV_8UC3);

	for (size_t y = 0; y < disp_height_; ++y)
	{
		cv::Vec3b *disp = display_.ptr<cv::Vec3b>(y);

		for (size_t x = 0; x < disp_width_; ++x)
		{
			size_t index = (floor(y / zoom_.y) + roi_.y) * layer_->width +
						   floor(x / zoom_.x) + roi_.x;

			int label = layer_->neurons[index].label;

			if (layer_->neurons[index].phase >= 0)
			{
				disp[x] = GetDisplayColor(label);
			}
			else
			{
				disp[x].val[0] = 0;
				disp[x].val[1] = 0;
				disp[x].val[2] = 0;
			}
		}
	}
}

//=============================================================================
//								Global Monitor
//=============================================================================
//list<shared_ptr<Monitor>> GlobalMonitor::monitors_;

// AddMonitor doesn't link when implemented in the source file (.cpp). All
// static functions are thus in the header file (.h)
////=============================================================================
//void GlobalMonitor::AddMonitor(shared_ptr<Monitor>& a_monitor)
//{
//	a_monitor->Display();
//	monitors_.push_back(a_monitor);
//}
//
////=============================================================================
//void DisplayImage(const string& a_title, const Mat& a_image)
//{
//	//namedWindow(title, CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO);
//	namedWindow(a_title, CV_GUI_EXPANDED|CV_WINDOW_KEEPRATIO);
//
//	// Display the image
//	imshow(a_title, a_image);
//
//	// Allow the program to display right away
//	waitKey(1);
//}