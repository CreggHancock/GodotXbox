#include <Godot.hpp>
#include <Node.hpp>
#include <OS.hpp>
#include "closeconsole.h"
#include <windows.ui.core.h>
#include <consoleapi.h>


using namespace godot;



void ConsoleCloser::_register_methods()
{

}

ConsoleCloser::ConsoleCloser()
{
}

ConsoleCloser::~ConsoleCloser()
{
}

void ConsoleCloser::_init()
{
	//check to see if we are using a debug build or not, and if it is a release build close the console
	OS* singleton = OS::get_singleton();
	if (singleton != nullptr && !singleton->is_debug_build())
	{
		Close();
	}
}


void ConsoleCloser::Close()
{
	//this is called in release builds to close the console as a temporary workaround for release builds
	//being forced to release as SUBSYSTEM/CONSOLE. This will close the console almost immediately.
	FreeConsole();
}