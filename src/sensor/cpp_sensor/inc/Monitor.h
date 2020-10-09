/** @file Monitor.h
 *
 * This files contains the Monitor class that is used to display images and
 * and layer data in pop-up windows.
 *
 *  @author Vincent de Ladurantaye
 */

#pragma once

#include <list>
#include <iostream>

#include "SegmentationLayer.h"

/**
* Display mode for the Monitor, this can be changed
*/
enum MonitorMode
{
	DSM_MONITOR_PIXELS		= 0,
	DSM_MONITOR_POTENTIAL,
	DSM_MONITOR_ORIENTATION,
	DSM_MONITOR_SEGMENTS,
	DSM_MONITOR_CUSTOM
};

/**
* Abstract class for displaying images that can be iteracted with in the 
* display window (zooming, click, etc.).
*/
class Monitor
{
public:
	/**
	* Constructor
	*/
	Monitor(const string& a_name);

	/**
	* Destructor
	*/
	virtual ~Monitor();

	/**
	* Displays the monitor in a window
	*/
	void Display();

	/**
	* Get the diplayed image
	*/
	cv::Mat GetDisplay();

	/**
	* Resets the size of the display to the size of the original image
	*/
	void ResetSize();

	/**
	* Redraws all monitors
	*/
	static void RefreshMonitors();

	/**
	* Callback function called for mouse actions in the display window
	*/
	static void MouseCallback(int a_event, int a_x, int a_y, 
							  int a_flags, void *a_data)
	{
		Monitor *self = static_cast<Monitor*>(a_data);
		self->MouseHandler(a_event, a_x, a_y, a_flags);
	}


protected:

	virtual void Draw() = 0;

	/**
	* Handles the mouse action in the display window. The default implementaion
	* calls the OnMouse...() virtual functions, to be implemented by child 
	* classes
	*/
	virtual void MouseHandler(int a_event, int a_x, int a_y, int a_flags) = 0;

	virtual void OnMouseClick(int a_event, int a_x, int a_y, int a_flags) = 0;

	virtual void OnMouseMove(int a_x, int a_y, int a_flags) = 0;

protected:
	// Title of the display window
	string title_;

	// Image actually displayed on screen
	Mat display_;

	// Size of the display window
	uint disp_width_;
	uint disp_height_;

	// Default/minimal size of the display window
	uint min_width_;
	uint min_height_;

	// Display zoom (window resize zoom)
	cv::Point2f disp_zoom_;

	// Flag indicating if the monitor has a parent monitor and should not
	// create it's own display window.
	bool is_child_;

	// List of all existing monitors
	static list<Monitor*> monitors_;
};

//=============================================================================
//								Image Monitor
//=============================================================================
/**
* Monitor for images. 
*/
class ImageMonitor: public Monitor
{
public:
	/**
	* Constructor
	*/
	ImageMonitor(const string& a_name, const Mat& a_image);

	/**
	* Virtual Destructor
	*/
	virtual ~ImageMonitor();

	void Draw() final;

	/**
	* Save to an image file
	*/
	void SaveAsImage(string a_filename);

protected:

	virtual void MouseHandler(int a_event, int a_x, int a_y, int a_flags);

	virtual void OnMouseClick(int a_event, int a_x, int a_y, int a_flags);

	virtual void OnMouseMove(int a_x, int a_y, int a_flags);

	/**
	* Draw the region of interest to be shown on the image with the current
	* zoom
	*/
	virtual void DrawRoi();

	/**
	* Draw lines to separate neurons when the zoom is high enough. Lines are
	* drawn using the given color.
	*/
	void DrawCells(cv::Scalar& a_color);

protected:

	// Clone of the original image
	Mat image_;

	// Section of the image currently displayed
	cv::Rect roi_;

	// Cursor dragging starting point
	cv::Point drag_start_;

	// Mouse scroll wheel zoom 
	float scroll_zoom_;

	// Final zoom, mouse wheel zoom + window resize zoom
	cv::Point2f zoom_;

	// Color for the cell delimitation lines
	cv::Scalar cell_color_;
};

//=============================================================================
//								Layer Monitor
//=============================================================================
/**
* Monitor for neural layers.
*/
class NeuralLayer;
class LayerMonitor: public ImageMonitor
{
public:
	/**
	* Constructor
	*/
	LayerMonitor(const string& a_name,
				 const NeuralLayer& a_layer,
				 MonitorMode a_mode = DSM_MONITOR_POTENTIAL);
	LayerMonitor(const string& a_name,
				 const NeuralLayer* a_layer, const ImageData& a_imageData,
				 MonitorMode a_mode = DSM_MONITOR_POTENTIAL);

	/**
	* Destructor
	*/
	virtual ~LayerMonitor();


	/**
	* Select the next display mode
	*/
	void SelectNextDisplayMode(bool a_select_prev);

	/**
	* Set the next display mode
	*/
	void SetDisplayMode(MonitorMode a_mode);

	/**
	* Get the display mode name
	*/
	static string GetDisplayModeName(MonitorMode a_mode);

protected:
	virtual void DrawRoi();
	
	virtual void OnMouseClick(int a_event, int a_x, int a_y, int a_flags);

	virtual cv::Vec3b GetDisplayColor(int a_val);

	void DrawCustom();

	void DrawPotential();

protected:
	// Image data of the image
	ImageData image_data_;

	// Reference to the observed layer
	const NeuralLayer* layer_;

	// Display mode
	MonitorMode mode_;

	// Display mode list to go through on CTRL+click
	vector<MonitorMode> mode_sequence_;

	friend class NetworkMonitor;
	friend class LayerCouplerMonitor;
};

//=============================================================================
//							  SegmentLayerMonitor
//=============================================================================
class SegmentationLayer;
class SegmentLayerMonitor: public LayerMonitor
{
public:
	/**
	* Constructor
	*/
	SegmentLayerMonitor(const string& a_name,
						const SegmentationLayer& a_layer,
						MonitorMode a_mode = DSM_MONITOR_SEGMENTS);
	SegmentLayerMonitor(const string& a_name,
						const SegmentationLayer* a_layer, 
						ImageData& a_image_data,
						MonitorMode a_mode = DSM_MONITOR_SEGMENTS);

	/**
	* Destructor
	*/
	virtual ~SegmentLayerMonitor();

protected:
	virtual void DrawRoi();

	virtual void OnMouseClick(int a_event, int a_x, int a_y, int a_flags);

	void DrawSegments();

};
