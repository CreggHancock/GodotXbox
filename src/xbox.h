#pragma once


#include <Godot.hpp>
#include <Node.hpp>
#include <xsapi/services.h>

namespace godot {

	class Xbox : public Node {
		GODOT_CLASS(Xbox, Node);
	private:
		
		void UpdateStatsManager();
		void TrySignInSilently();
		void SignInSuccessful();
		void SignInFailed();
		void AddSignOut();
		void AddUserToStatsManager(_In_ std::shared_ptr<xbox::services::system::xbox_live_user>& user);
		void RemoveUserFromStatsManager(_In_ std::shared_ptr<xbox::services::system::xbox_live_user>& user);
		void ProcessLeaderboards(_In_ xbox::services::xbox_live_result<xbox::services::leaderboard::leaderboard_result> result);
		void UpdateLeaderboardEntries(_In_ std::vector<xbox::services::leaderboard::leaderboard_row> rows);

	public:
		static void _register_methods();

		Xbox();
		~Xbox();

		bool IsSignedIn();
		void SignInManually();
		String GetGamertag();

		
		Variant GetStatForUser(String statName);
		void DeleteStatForUser(String statName);
		void QueryLeaderboard(String statName, int maxItems);
		void QueryLeaderboardSkipToRank(String statName, int rank, int maxItems);
		void QueryLeaderboardSkipToSelf(String statName, int maxItems);
		void QueryLeaderboardForSocialGroup(String statName, String socialGroup, int maxItems);
		
		Array GetLeaderboardEntries();
		
		void SetStatForUser(String statName, Variant statValue);

		void _init(); // our initializer called by Godot
		void _ready();
		void _process(float delta);

		std::shared_ptr<xbox::services::system::xbox_live_user> oMyUser; //currently signed in xbox user pointer
		std::shared_ptr<xbox::services::xbox_live_context> xboxLiveContext; //used with certain live features such as leaderboards, etc
		std::wstring strLoginErrors;

		std::shared_ptr<xbox::services::stats::manager::stats_manager> mStatsManager;

		Array leaderboardEntries;

	};

}

