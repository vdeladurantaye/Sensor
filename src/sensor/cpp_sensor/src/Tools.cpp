///** @file Utility.cpp
// *
// *  @author Vincent de Ladurantaye
// *  @date 02/2010
// */
//#include "Tools.h"
//
//#include <fstream>
//#include <iostream>
//using namespace std;
//
////=============================================================================
//void Utility::SaveGrayscaleImg(
//    string file_name,
//	float* data,
//	int rows, int cols)
//{
//	// Open the output file
//	ofstream outFile(file_name, ios::out);
//
//	// Write the header to file
//	outFile << "P2" << endl;
//	outFile << cols << " " << rows << endl;
//	outFile << "255" << endl;
//
//	// Processes data to create output file
//	int index;
//	for (int i = 0; i < rows; ++i)
//	{
//		for (int j = 0; j < cols; ++j)
//		{
//			index = i*cols+j;
//			outFile << " " << (int)data[index];
//		}
//		outFile << endl;
//	}
//
//	// Close the file
//	outFile.close();
//}
//
////=============================================================================
//void Utility::RotateCoordinates(
//    cv::Point2f& point, 
//	cv::Mat& rotation_mat,
//	cv::Point2f& new_point)
//{
//	// New x position
//	new_point.x = rotation_mat.at<double>(0,0) * point.x 
//		+ rotation_mat.at<double>(0,1) * point.y 
//		+ rotation_mat.at<double>(0,2);
//	// New y position
//	new_point.y = rotation_mat.at<double>(1,0)*point.x 
//		+ rotation_mat.at<double>(1,1)*point.y 
//		+ rotation_mat.at<double>(1,2);
//}
//
////=============================================================================
//void Utility::RotateCoordinates(
//    cv::Point2f& point, 
//	float angle,
//	cv::Point2f& new_point)
//{
//	// Get rotation matrix
//	Mat rot_mat(2, 3, CV_32FC1);
//	rot_mat = getRotationMatrix2D(cv::Point2f(0,0),angle,1);
//
//	RotateCoordinates(point, rot_mat, new_point);
//}
