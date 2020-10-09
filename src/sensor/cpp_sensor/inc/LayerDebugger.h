/** @file LayerDebugger.h
*
* Debugger for steping into the network at different levels. Uses monitors to
* display the state of neural layers.
*
*  @author Vincent de Ladurantaye
*/

#pragma once

// Comment the LAYER_DEBUGGER definition to deactivate the debugger
#define LAYER_DEBUGGER

#include <list>
#include <memory>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <string>

#include "Monitor.h"

/**
* Debug levels at which we can step in the code.
*/
enum DebugLevel
{
	DEBUG_LEVEL_SPIKE,
	DEBUG_LEVEL_CASCADE,
	DEBUG_LEVEL_CYCLE,
	DEBUG_LEVEL_END
};

// Forward declaration
class NeuralLayer;

/**
* Class for debugging neural layers. A debugger is created when calling 
* AddDebugger(). Breakpoints are placed in the code using SetBreakpoint(). 
*
* Calls to the debugger are static so that they can be made from anywhere and
* that the debugger objects don't have to be handled in the code.
*
* Different debug levels are used to identify different nesting levels. For 
* example, if the global debug level is "CASCADE", all "SPIKE" level 
* calls will be skipped.
*
* Unless displaying in real-time the function will wait for a key to be
* pressed. The global debug level can be changed with different keys:
*  Key	  Level					Description
*	n:	1-(spike)		- Steps between each spike
*	s:	2-(cascade)		- Steps between each firing step/cascade
*	c:	3-(cycle)		- Steps between each firing cycle
*	e:	4-(end)			- Steps to the end of the simulation
*
*	r:	Toggle real-time display
*/
class LayerDebugger
{
public:
	/**
	* Destructor
	*/
	~LayerDebugger();

	/**
	* Adds a debugger for the given layer
	*/
	static void AddDebugger(const NeuralLayer& layer, const std::string& name);

	/**
	* Displays the help message showing the debugger commands in the terminal
	*/
	static void DisplayHelp();

	/**
	* Places a breakpoint in the code. This function needs to be called at 
	* location where we want the debugger to stop. 
	*
	* @param layer Layer from which the call is made
	* @param debugLvl Specifies the debug level for which the function
	*		 will wait when called. 
	* @param progressId Number indicating the current progress in the 
	*		 simulation. This is displayed in the console when breaking.
	*/
	static void SetBreakpoint(const NeuralLayer& layer,
							  DebugLevel debugLvl, 
							  int progressId = -1);

	/**
	* Changes the debug level to the given value.

	* DEBUG_LEVEL_SPIKE: Steps between each spike
	* DEBUG_LEVEL_CASCADE: Steps between each firing cascade
	* DEBUG_LEVEL_CYCLE: Steps between each firing cycle
	* DEBUG_LEVEL_END: Steps to the end of the simulation
	*/
	static void SetDebugLvl(DebugLevel a_debugLvl);
	
	/**
	* Toggle real time display so that the programs doesn't wait for the user
	* to press a key between each breakpoint.
	*/
	static void ToggleRealTimeDisplay(bool a_real_time);

	/**
	* If multithreading, this is called by the UI thread to wait for the worker
	* threads. The UI thread will wait for notifications from worker threads
	* to update monitor displays.
	*/
	static void WaitForWorkerThreads();


	//-------------------------------------------------------------------------
	//							 Non-Static members
	//-------------------------------------------------------------------------
	// These non-static members are specific to LayerDebugger objects which
	// are created by AddDebugger(). Each object represents a layer being
	// debugged.
private:
	/**
	* Function called by SetBreakpoint() on the object corresponding to the 
	* layer that set the breakpoint.
	*/
	void OnWait(DebugLevel debugLvl, int progressId);

private:
	// Reference to the layer being debugged
	const NeuralLayer& layer_;

	// Monitor for viewing layer state
	LayerMonitor monitor_;

	// Name of the layer used for outputs in the console
	std::string name_;

	// Flag used in multitreading to indicate the layer is ready to display
	bool thread_ready_;

	// Flag indicating if the layer is done with it's work. When all the layers
	// set this flag to true, the UI thread will exit WaitForWorkerThreads().
	//
	// This is automatically set to true when reaching a breakpoint with a 
	// debug level DEBUG_LEVEL_END.
	bool work_done_;

	//-------------------------------------------------------------------------
	//							 Static members
	//-------------------------------------------------------------------------
private:
	/**
	* Constructor. Private because AddDebugger() needs to be called to add a
	* debugger.
	*/
	LayerDebugger(const NeuralLayer& layer, const std::string& name);

	/**
	* Function called by WaitForWorkerThreads() to check if all worker threads
	* are ready to display.
	*/
	static bool AreWorkerTreadsReady();

	/**
	* Function called by WaitForWorkerThreads() to check if all worker threads
	* are done with their work.
	*/
	static bool AreWorkerTreadsDone();

	/**
	* Refreshes the monitors and wait for a given amount of time, allowing the
	* windows to refresh. If wait time is 0, waits until a key is pressed.
	*/
	static void RefreshAndWait();

    
    // Fix for compiling in XCODE, which doesn't seem to support lambdas
    static bool CondVariableFunctor() { return ui_ready_; }

private:
	// Current debug level
	static DebugLevel debug_lvl_;

	// Flag indicating if debugging steps are displayed in real-time
	static bool real_time_display_;

	// Global list of debuggers
	static std::list<std::shared_ptr<LayerDebugger> > debuggers_;

	// Id of the UI thread. We save it as only the UI thread can interact with
	// monitors.
	static std::thread::id ui_thread_id_;

	static std::mutex display_mutex_;
	static std::condition_variable display_condition_;

	// Flag used in multithreading to indicate that the UI thread is done 
	// displaying
	static bool ui_ready_;
};
