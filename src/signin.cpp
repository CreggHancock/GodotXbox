#include <Godot.hpp>
#include <Node.hpp>
#include <String.hpp>
#include "signin.h"
#include "xsapi\services.h"
#include <codecvt>
#include <concrt.h>
#include <windows.ui.core.h>


using namespace godot;



void SignIn::_register_methods()
{
	register_method("_ready", &SignIn::_ready);
	register_method("SignInManually", &SignIn::SignInManually);
	register_method("GetGamertag", &SignIn::GetGamertag);
	register_method("IsSignedIn", &SignIn::IsSignedIn);

	register_signal<SignIn>("signed_out", Dictionary());
	register_signal<SignIn>("signed_in", Dictionary());
	register_signal<SignIn>("signin_failed", Dictionary());
}

SignIn::SignIn()
{
	oMyUser = nullptr;
	strLoginErrors = L"Starting... ";
}

SignIn::~SignIn()
{

}

void SignIn::_init()
{
	oMyUser = nullptr;
	strLoginErrors = L"Starting... ";
}

void SignIn::_ready()
{
	TrySignInSilently();
}


void SignIn::SignInManually()
{
	// ***** Signing to Xbox Live... ***** //
	//once the result is returned from the other thread, emit a signal with the sign in result
	try
	{
		if (oMyUser != nullptr && !oMyUser->is_signed_in())
		{

			Concurrency::create_task(oMyUser->signin(Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher))
				.then([this](xbox::services::xbox_live_result<xbox::services::system::sign_in_result> oMyResult)
					{
						if (!oMyResult.err())
						{
							switch (oMyResult.payload().status())
							{
								case xbox::services::system::sign_in_status::success:
									strLoginErrors = L"Success";
									SignInSuccessful();
									break;
								case xbox::services::system::sign_in_status::user_cancel:
									strLoginErrors = L"cancelled";
									SignInFailed();
									break;
								default:
									strLoginErrors = L"don't know";
									SignInFailed();
									break;
							}
						}
						else
						{
							wchar_t chrValue[200];
							_itow_s(oMyResult.err().value(), chrValue, 200, 16);
							strLoginErrors = std::wstring(chrValue);
							SignInFailed();
						}
					});
		}
		else if (oMyUser == nullptr)
		{
			//this is a catch for if a user signed out. we try signing in silently again to create a new oMyUser
			TrySignInSilently();
		}
	}
	catch (std::exception oMyEx)
	{
		strLoginErrors = std::wstring_convert<std::codecvt_utf8<char_t>>().from_bytes(oMyEx.what());
		SignInFailed();
	}
}

void SignIn::TrySignInSilently()
{
	// ***** Signing to Xbox Live... ***** //
	//create an xbox_live_user and once the result is returned from the other thread, emit a signal with the sign in result
	try
	{
		oMyUser = std::make_shared<xbox::services::system::xbox_live_user>();
		if (oMyUser != nullptr)
		{
			auto oMyAsyncOp = oMyUser->signin_silently(Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher);
			create_task(oMyAsyncOp)
				.then([this](xbox::services::xbox_live_result<xbox::services::system::sign_in_result> oMyResult)
					{
						if (!oMyResult.err())
						{
							switch (oMyResult.payload().status())
							{
								case xbox::services::system::sign_in_status::success:
									strLoginErrors = L"Success";
									SignInSuccessful();
									break;
								case xbox::services::system::sign_in_status::user_cancel:
									strLoginErrors = L"cancelled";
									SignInFailed();
									break;
								default:
									strLoginErrors = L"interaction required";
									SignInFailed();
									break;
							}
						}
						else
						{
							wchar_t chrValue[200];
							_itow_s(oMyResult.err().value(), chrValue, 200, 16);
							strLoginErrors = std::wstring(chrValue);
							SignInFailed();
						}
					});
		}
	}
	catch (std::exception oMyEx)
	{
		strLoginErrors = std::wstring_convert<std::codecvt_utf8<char_t>>().from_bytes(oMyEx.what());
		SignInFailed();
	}
}

bool SignIn::IsSignedIn()
{
	//Is our xbox_live_user signed in?
	if (oMyUser != nullptr)
	{
		return (oMyUser->is_signed_in());
	}
	return false;
}

String SignIn::GetGamertag()
{
	//Returns the currently signed in user's gamertag. 
	//If there is no user, return the current strLoginErrors status. **This will likely change soon as it was used for debugging**
	//Regardless, you should check if you are signed in by calling IsSignedIn() first so you don't accidentally display an error as a user.
	try
	{
		if (IsSignedIn())
		{
			std::wstring username = (oMyUser->gamertag());
			const wchar_t* wcs = username.c_str();
			return String(wcs);
		}
		const wchar_t* no_gamertag = strLoginErrors.c_str();
		return String(no_gamertag);
	}
	catch (...)
	{
		strLoginErrors = L"getgamertag failed";
		const wchar_t* gamertag_failed = strLoginErrors.c_str();
		return String(gamertag_failed);
	}
}

void SignIn::SignInSuccessful()
{
	//Sign in was successful, emits a signal to inform the game.
	xboxLiveContext = std::make_shared<xbox::services::xbox_live_context>(oMyUser);
	AddSignOut();
	emit_signal("signed_in", Dictionary());
}

void SignIn::SignInFailed()
{
	//Sign in failed due to a number of possible reasons, emits a signal. You should try signing in manually when this happens.
	emit_signal("signin_failed", Dictionary());
}

void SignIn::AddSignOut()
{
	//The user signed out, emits a signal. You should try signing in again after this.
	oMyUser->add_sign_out_completed_handler(
		[this](const xbox::services::system::sign_out_completed_event_args&)
	{
		xboxLiveContext = nullptr;
		emit_signal("signed_out", Dictionary());
	});
}
