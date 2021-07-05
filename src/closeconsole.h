#pragma once

#include <Godot.hpp>
#include <Node.hpp>

namespace godot {

	class ConsoleCloser : public Node {
		GODOT_CLASS(ConsoleCloser, Node);
		

	public:
		static void _register_methods();

		ConsoleCloser();
		~ConsoleCloser();

		void _init(); // our initializer called by Godot

		void Close();
	};

}

