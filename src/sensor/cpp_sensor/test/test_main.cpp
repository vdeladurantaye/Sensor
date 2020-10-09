/** @file test_main.cpp
 *
 * Main test file for gtest
 *
 *  @author Vincent de Ladurantaye
 */

#include <iostream>
using namespace std;

#include "gtest/gtest.h"


int main(int  argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);

	// Select a specific subset of tests that will be executed
	::testing::GTEST_FLAG(filter) = "TestOdlmPixel*";

	return RUN_ALL_TESTS();

	return 0;
}