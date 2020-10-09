/** @file test_odlm.h
*
* @authors Vincent de Ladurantaye
*/
#pragma once

#include "gtest/gtest.h"

#include "NeuralNetwork.h"


//=============================================================================
//								TestOdlmPixel
//=============================================================================
/**
* Class for testing the pixel ODLM working with gray pixel values.
*/
class TestOdlmPixel: public ::testing::Test
{
public:
	/**
	* Constructor
	*/
	TestOdlmPixel();


	/**
	* Code here will be called immediately after the constructor (right
	* before each test).
	*/
	virtual void SetUp();

	/**
	* Code here will be called immediately after each test (right
	* before the destructor).
	*/
	virtual void TearDown();
	
};
