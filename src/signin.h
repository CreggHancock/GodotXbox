#pragma once


#include <Godot.hpp>
#include <Node.hpp>
#include <xsapi/services.h>

namespace godot {

	class SignIn : public Node {
	
	private:
		GODOT_CLASS(SignIn, Node);

	public:
		static void _register_methods();

		SignIn();
		~SignIn();

		bool IsSignedIn();
		void SignInManually();
		void TrySignInSilently();
		void SignInSuccessful();
		void SignInFailed();
		void AddSignOut();
		String GetGamertag();

		void _init(); // our initializer called by Godot
		void _ready();

		std::shared_ptr<xbox::services::system::xbox_live_user> oMyUser;
		std::shared_ptr<xbox::services::xbox_live_context> xboxLiveContext;
		std::wstring strLoginErrors;

	};

}

