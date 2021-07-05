#include <Godot.hpp>
#include <Node.hpp>
#include <String.hpp>
#include "xbox.h"
#include "xsapi\services.h"
#include <stats_manager.h>
#include <codecvt>
#include <concrt.h>
#include <windows.ui.core.h>
#include <windows.gaming.xboxlive.storage.h>


using namespace godot;
using namespace xbox::services::stats::manager;
using namespace xbox::services;
using namespace Windows::Gaming::XboxLive::Storage;



void Xbox::_register_methods()
{
	register_method("_ready", &Xbox::_ready);
	register_method("_process", &Xbox::_process);
	register_method("SignInManually", &Xbox::SignInManually);
	register_method("GetGamertag", &Xbox::GetGamertag);
	register_method("IsSignedIn", &Xbox::IsSignedIn);
	register_method("SetStat", &Xbox::SetStatForUser);
	register_method("DeleteStat", &Xbox::DeleteStatForUser);
	register_method("QueryLeaderboard", &Xbox::QueryLeaderboard);
	register_method("QueryLeaderboardSkipToRank", &Xbox::QueryLeaderboardSkipToRank);
	register_method("QueryLeaderboardSkipToSelf", &Xbox::QueryLeaderboardSkipToSelf);
	register_method("QueryLeaderboardForSocialGroup", &Xbox::QueryLeaderboardForSocialGroup);
	register_method("GetLeaderboardEntries", &Xbox::GetLeaderboardEntries);
	register_method("GetStat", &Xbox::GetStatForUser);

	register_signal<Xbox>("signed_out", Dictionary());
	register_signal<Xbox>("signed_in", Dictionary());
	register_signal<Xbox>("signin_failed", Dictionary());
	register_signal<Xbox>("leaderboards_ready", Dictionary());
	register_signal<Xbox>("stat_user_added", Dictionary());
	register_signal<Xbox>("stat_user_removed", Dictionary());
	register_signal<Xbox>("player_stats_updated", Dictionary());
}

Xbox::Xbox()
{
	oMyUser = nullptr;
	mStatsManager = nullptr;
	strLoginErrors = L"Starting... ";
}

Xbox::~Xbox()
{

}

void Xbox::_init()
{
	oMyUser = nullptr;
	strLoginErrors = L"Starting... ";
	mStatsManager = stats_manager::get_singleton_instance();
}

void Xbox::_ready()
{
	TrySignInSilently();
}

void Xbox::_process(float delta)
{
	UpdateStatsManager();
}

//Authentication

void Xbox::SignInManually()
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

void Xbox::TrySignInSilently()
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

bool Xbox::IsSignedIn()
{
	//Is our xbox_live_user signed in?
	if (oMyUser != nullptr)
	{
		return (oMyUser->is_signed_in());
	}
	return false;
}

String Xbox::GetGamertag()
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

void Xbox::SignInSuccessful()
{
	//Sign in was successful, emits a signal to inform the game.
	xboxLiveContext = std::make_shared<xbox::services::xbox_live_context>(oMyUser);
	AddUserToStatsManager(oMyUser);
	AddSignOut();
	emit_signal("signed_in", Dictionary());
}

void Xbox::SignInFailed()
{
	//Sign in failed due to a number of possible reasons, emits a signal. You should try signing in manually when this happens.
	emit_signal("signin_failed", Dictionary());
}

void Xbox::AddSignOut()
{
	//The user signed out, emits a signal. You should try signing in again after this.
	oMyUser->add_sign_out_completed_handler(
		[this](const xbox::services::system::sign_out_completed_event_args&)
	{
		RemoveUserFromStatsManager(oMyUser);
		xboxLiveContext = nullptr;
		emit_signal("signed_out", Dictionary());
	});
}



//Leaderboards and Stats

void Xbox::AddUserToStatsManager(_In_ std::shared_ptr<xbox::services::system::xbox_live_user> &user)
{
	//Adds a user to our stats manager
	mStatsManager->add_local_user(user);
}

void Xbox::RemoveUserFromStatsManager(_In_ std::shared_ptr<xbox::services::system::xbox_live_user> &user)
{
	//Removes user from our stats manager
	mStatsManager->remove_local_user(user);
}

void Xbox::SetStatForUser(String statName, Variant statValue)
{
	//set a user's stat. Strings are't supported yet, but you can submit floats (REAL) or integers
	if (!IsSignedIn()) { return; }

	auto cName = statName.unicode_str();

	switch (statValue.get_type())
	{
		case Variant::Type::INT:
			mStatsManager->set_stat_as_integer(oMyUser, cName, statValue);
			break;
		case Variant::Type::REAL:
			mStatsManager->set_stat_as_number(oMyUser, cName, statValue);
			break;
		case Variant::Type::STRING:
			//not supporting strings for now
			break;
	}

	// Typically stats will be uploaded automatically
	// you should only request to flush when a game session or level ends.
	mStatsManager->request_flush_to_service(oMyUser, false);
}

void Xbox::DeleteStatForUser(String statName)
{
	//Delete user stat
	if (!IsSignedIn()) { return; }

	auto cName = statName.unicode_str();

	mStatsManager->delete_stat(oMyUser, cName);
}

void Xbox::QueryLeaderboard(String statName, int maxItems = 0)
{
	//Try to fetch the leaderboard with the given name, payload will be received in UpdateStatsManager
	if (!IsSignedIn()) { return; }

	auto cName = statName.unicode_str();

	leaderboard::leaderboard_query leaderboardQuery;
	if (maxItems > 0)
	{
		leaderboardQuery.set_max_items(maxItems);
	}
	mStatsManager->get_leaderboard(oMyUser, cName, leaderboardQuery);
}

void Xbox::QueryLeaderboardSkipToRank(String statName, int rank, int maxItems = 0)
{
	//Try to fetch the leaderboard with the given name and skips to rank in leaderboard, payload will be received in UpdateStatsManager
	if (!IsSignedIn()) { return; }

	auto cName = statName.unicode_str();

	xbox::services::leaderboard::leaderboard_query leaderboardQuery;
	if (maxItems > 0)
	{
		leaderboardQuery.set_max_items(maxItems);
	}
	leaderboardQuery.set_skip_result_to_rank(rank);
	mStatsManager->get_leaderboard(oMyUser, cName, leaderboardQuery);
}

void Xbox::QueryLeaderboardSkipToSelf(String statName, int maxItems = 0)
{
	//Try to fetch the leaderboard with the given name and skips to current user's position, payload will be received in UpdateStatsManager
	if (!IsSignedIn()) { return; }

	auto cName = statName.unicode_str();

	xbox::services::leaderboard::leaderboard_query leaderboardQuery;
	if (maxItems > 0)
	{
		leaderboardQuery.set_max_items(maxItems);
	}
	leaderboardQuery.set_skip_result_to_me(true);
	mStatsManager->get_leaderboard(oMyUser, cName, leaderboardQuery);
}

void Xbox::QueryLeaderboardForSocialGroup(String statName, String socialGroup, int maxItems = 0)
{
	//Try to fetch the leaderboard with the given name and xbox live social group payload will be received in UpdateStatsManager
	if (!IsSignedIn()) { return; }

	auto cName = statName.unicode_str();
	auto cSocialGroup = socialGroup.unicode_str();

	xbox::services::leaderboard::leaderboard_query leaderboardQuery;
	if (maxItems > 0)
	{
		leaderboardQuery.set_max_items(maxItems);
	}
	mStatsManager->get_social_leaderboard(oMyUser, cName, cSocialGroup, leaderboardQuery);
}

void Xbox::UpdateStatsManager()
{
	if (!IsSignedIn()) { return; }
	// Process events from the stats manager
	// This is called each frame update


	auto statsEvents = mStatsManager->do_work();
	std::wstring text;

	for (const auto& evt : statsEvents)
	{
		switch (evt.event_type())
		{
			case stat_event_type::local_user_added:
				//user was added, emits signal to inform game that it is safe to get/set stats
				emit_signal("stat_user_added", Dictionary());
				break;

			case stat_event_type::local_user_removed:
				//user was removed, emits signal to inform game that it is no longer safe to get/set stats
				emit_signal("stat_user_removed", Dictionary());
				break;

			case stat_event_type::stat_update_complete:
				//player stats were successfully updated from a previous SetStat call
				emit_signal("player_stats_updated", Dictionary());
				break;

			case stat_event_type::get_leaderboard_complete:
				//leaderboard query came back, process the leaderboard so it can be converted to a Godot Array and given to the game
				auto getLeaderboardCompleteArgs = std::dynamic_pointer_cast<leaderboard_result_event_args>(evt.event_args());
				ProcessLeaderboards(getLeaderboardCompleteArgs->result());
				break;
		}
	}
	
}

void Xbox::ProcessLeaderboards(_In_ xbox::services::xbox_live_result<xbox::services::leaderboard::leaderboard_result> result)
{
	if (!IsSignedIn()) { return; }

	if (!result.err())
	{
		//if there is no error, conver leaderboardEntries into a Godot array and send them off in signal
		auto leaderboardResult = result.payload();

		std::vector<xbox::services::leaderboard::leaderboard_row> leaderboardRows = leaderboardResult.rows();
		UpdateLeaderboardEntries(leaderboardRows);
		Dictionary signalArgs;
		signalArgs["entries"] = leaderboardEntries;
		emit_signal("leaderboards_ready", signalArgs);


		// Keep processing if there is more data in the leaderboard
		if (leaderboardResult.has_next())
		{
			if (!leaderboardResult.get_next_query().err())
			{
				auto lbQuery = leaderboardResult.get_next_query().payload();
				if (lbQuery.social_group().empty())
				{
					mStatsManager->get_leaderboard(oMyUser, lbQuery.stat_name(), lbQuery);
				}
				else
				{
					mStatsManager->get_social_leaderboard(oMyUser, lbQuery.stat_name(), lbQuery.social_group(), lbQuery);
				}
			}
		}
	}
	else
	{
		Godot::print(result.err().message().c_str());
	}
}

void Xbox::UpdateLeaderboardEntries(_In_ std::vector<xbox::services::leaderboard::leaderboard_row> rows)
{
	//clear entries, fill up a Godot Array with new data, and cache that data for polling
	leaderboardEntries.clear();
	int vectorSize = rows.size();
	if (vectorSize > 0)
	{
		for (int i = 0; i < vectorSize; i++) {
			Dictionary entryDict;
			xbox::services::leaderboard::leaderboard_row entry = rows[i];
			string_t colValues;
			for (auto columnValue : entry.column_values())
			{
				colValues = colValues + L" ";
				colValues = colValues + columnValue;
			}
			entryDict["score"] = String(colValues.c_str());
			entryDict["ID"] = String(entry.xbox_user_id().c_str());
			entryDict["rank"] = entry.rank();
			entryDict["gamerTag"] = String(entry.gamertag().c_str());
			leaderboardEntries.append(entryDict);
		}
	} 
	else
	{
		Godot::print("entry size is 0");
	}
}

Array Xbox::GetLeaderboardEntries()
{
	//Call this to get the currently updated leaderboard entries
	return leaderboardEntries;
}

Variant Xbox::GetStatForUser(String statName)
{
	if (!IsSignedIn()) { return -1; }
	//Gets a user's stat and returns it as a Godot Variant

	auto cName = statName.unicode_str();
	const string_t& statQuery = cName;

	xbox::services::xbox_live_result<stat_value> statResult = mStatsManager->get_stat(oMyUser, statQuery);
	stat_value stat = statResult.payload();
	switch (stat.data_type())
	{
		case stat_data_type::number:
			return stat.as_number();
		case stat_data_type::string:
			return stat.as_string().c_str();
		case stat_data_type::undefined:
			return L"undefined";
	}

	return -1;
}
