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
	// add your cleanup here
}

void ConsoleCloser::_init()
{
	OS* singleton = OS::get_singleton();
	if (singleton != nullptr && !singleton->is_debug_build())
	{
		Close();
	}
}


void ConsoleCloser::Close()
{
	FreeConsole();
}