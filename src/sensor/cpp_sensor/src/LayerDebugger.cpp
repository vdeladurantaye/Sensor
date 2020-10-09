/** @file LayerDebugger.cpp
*
*  @author Vincent de Ladurantaye
*/

#include "LayerDebugger.h"

#include "NeuralLayer.h"


#include <iostream>
using namespace std;

//=============================================================================
//						Static members declarations
//=============================================================================
// Static list of debuggers
list<shared_ptr<LayerDebugger> > LayerDebugger::debuggers_;
// Current debug level
DebugLevel LayerDebugger::debug_lvl_ = DEBUG_LEVEL_CASCADE;
// Real-time display flag
bool LayerDebugger::real_time_display_ = false;
// UI thread ID
thread::id LayerDebugger::ui_thread_id_ = this_thread::get_id();
// Mutex for diplay control
mutex LayerDebugger::display_mutex_;
// Condition variable for display control
condition_variable LayerDebugger::display_condition_;
// Flag for UI thread display status
bool LayerDebugger::ui_ready_ = false;

//=============================================================================
LayerDebugger::LayerDebugger(const NeuralLayer& layer, 
							 const std::string& name):
	layer_(layer),
	monitor_(name, layer),
	name_(name),
	thread_ready_(false),
	work_done_(false)
{
	monitor_.Display();

	// Display the help msg one time at the start of the simulation
	static bool initialHelpDisplayed = false;
	if(!initialHelpDisplayed)
	{
		DisplayHelp();
		initialHelpDisplayed = true;
	}
}

//=============================================================================
LayerDebugger::~LayerDebugger()
{
}

//=============================================================================
void LayerDebugger::AddDebugger(const NeuralLayer& layer, 
								const std::string& name)
{
	// Add a new debugger to the list
	debuggers_.push_back(shared_ptr<LayerDebugger>(
		new LayerDebugger(layer, name)));
}

//=============================================================================
void LayerDebugger::DisplayHelp()
{
	cout << "Layer Debugger\n"
		<< "Press the following keys to step between events\n"
		<< "Key\t\tDescription\n"
		<< "n:\t\t-Steps between each neuron spike\n"
		<< "s:\t\t-Steps between each firing step/cascade\n"
		<< "c:\t\t-Steps between each firing cycle\n"
		<< "e:\t\t-Steps to the end of the simulation\n"
		<< "r:\t\t-Toggle realtime display\n\n";
}

//=============================================================================
void LayerDebugger::SetBreakpoint(const NeuralLayer& layer,
								  DebugLevel debugLvl,
								  int progressId)
{
	// Find the debugger for this layer
	for (auto& debugger: debuggers_)
	{
		if (debugger->layer_.layer_id == layer.layer_id)
			debugger->OnWait(debugLvl, progressId);
	}
}

//=============================================================================
void LayerDebugger::SetDebugLvl(DebugLevel a_debugLvl)
{
	debug_lvl_ = a_debugLvl;
}

//=============================================================================
void LayerDebugger::ToggleRealTimeDisplay(bool a_real_time)
{
	real_time_display_ = a_real_time;
}

//=============================================================================
void LayerDebugger::WaitForWorkerThreads()
{
#ifdef LAYER_DEBUGGER
	while (true)//AreWorkerTreadsDone() == false)
	{
		unique_lock<mutex> lock(display_mutex_);
	//	cout << "UI waiting for threads..." << endl;
		display_condition_.wait(lock, LayerDebugger::AreWorkerTreadsReady);

		if (AreWorkerTreadsDone()) return;

	//	cout << "UI refresing display..." << endl << endl;
		RefreshAndWait();
		
		// Set the thread ready status back to false before notifiying them
		// to continue with their work
		for (auto& debugger: debuggers_)
		{
			debugger->thread_ready_ = false;
		}
		// Notify the threads
		ui_ready_ = true;
		lock.unlock();
		display_condition_.notify_all();
	}
#endif
}

//=============================================================================
bool LayerDebugger::AreWorkerTreadsReady()
{
	// Check if all layer debuggers are ready
	for (auto& debugger: debuggers_)
	{
		if (debugger->thread_ready_ == false) return false;
	}
	return true;
}

//=============================================================================
bool LayerDebugger::AreWorkerTreadsDone()
{
	// Check if all debugged layer are done
	for (auto& debugger: debuggers_)
	{
		if (debugger->work_done_ == false) return false;
	}
	return true;
}

//=============================================================================
void LayerDebugger::RefreshAndWait()
{
	Monitor::RefreshMonitors();

	int waitTime = 0;
	if (real_time_display_) waitTime = 1;
	// Wait for a key
	int k = cv::waitKey(waitTime);

	// Key press was "n"
	if (k == 'n') debug_lvl_ = DEBUG_LEVEL_SPIKE;
	// Key press was "i"
	if (k == 's') debug_lvl_ = DEBUG_LEVEL_CASCADE;
	// Key press was "c"
	if (k == 'c') debug_lvl_ = DEBUG_LEVEL_CYCLE;
	// Key press was "e"
	if (k == 'e') debug_lvl_ = DEBUG_LEVEL_END;

	// Key press was "r"
	if (k == 'r') real_time_display_ = !real_time_display_;
}

//=============================================================================
void LayerDebugger::OnWait(DebugLevel debugLvl, int progressId)
{
	// Return immediatly if not at the correct level and not at the end
	if ((debugLvl != debug_lvl_ && debugLvl != DEBUG_LEVEL_END))
		return;

	// If in the right debug level
	if (debug_lvl_ == debugLvl)
	{
		// Lock mutex for not mixing up console display
		lock_guard<mutex> lk(display_mutex_);

		// Display the debug level
		cout << name_;
		switch (debugLvl)
		{
		case DEBUG_LEVEL_SPIKE:
			cout << " Neuron: " << progressId << endl;
			break;

		case DEBUG_LEVEL_CASCADE:
			cout << " Cascade: " << progressId << endl;
			break;

		case DEBUG_LEVEL_CYCLE:
			cout << " Cycle: " << progressId << endl;
			break;

		case DEBUG_LEVEL_END:
			cout << " done!" << endl;
			// If the last call, set the work_done_ flag
			work_done_ = true;
			break;
		}
	}

	// If from the UI trhead, display and wait for key
	if (ui_thread_id_ == this_thread::get_id())
	{
		RefreshAndWait();
	}
	else // We are in a worker thread. 
	{
		// Notify the UI thread for display
		{
			lock_guard<mutex> lk(display_mutex_);
			// Set the flags
			thread_ready_ = true;
			ui_ready_ = false;
		//	cout << this_thread::get_id()
		//		 << " Thread done, notifying UI..." << endl;
		}
		display_condition_.notify_all();
        
		// Wait for the UI
		if (!work_done_)
		{
			unique_lock<mutex> lk(display_mutex_); 
			//cout << this_thread::get_id()
			//	 << " Thread waiting for UI..." << endl;
            
			// Fix for compiling in XCODE, which doesn't seem to support lambdas
            display_condition_.wait(lk, CondVariableFunctor);
            //display_condition_.wait(lk, [] { return ui_ready_; });
		}
	}
}
