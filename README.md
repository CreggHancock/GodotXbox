# GodotXbox

##### Current instructions for setting up DLL, steps to setup project for contributing/modifying are coming soon.

*[It should be noted that Godot's UWP export is currently not in a stable state](https://github.com/godotengine/godot/issues?q=is%3Aissue+is%3Aopen+uwp). There are work-arounds for many of the problems, but you may encounter some
bugs that do not have workarounds. Please report such issues to Godot's official repository.*

## Setup
1. Add the godot_xbox dll to your project.
2. Setup a gdnlib file to link to the dll.
3. Setup a gdns file fo both the Xbox class and ConsoleCloser class. 
4. Create instances of nodes for both classes. These can be autoloads or instanced in a scene that wont be going away.

GodotXbox will attempt to sign in silently once the node hits its _ready(). If a silent sign-in fails you will be required to tell the DLL to sign in manually. 

## Reference

### Methods

- **void** `SignInManually()` Call this to tell windows to attempt to sign you in manually. If you are not already signed in, this should create a popup asking you to sign in.
- **bool** `IsSignedIn()` Returns whether the current user is signed in. If there is no user, returns false.
- **String** `GetGamertag()` Returns the currently signed in user's gamertag if a user is signed in, otherwise returns an error message.
- **void** `SetStat(String statName, Variant statValue)` Set the currently signed in user's stat. Use the leaderboard stat ID created in Partner Center. Valid value types for statValue at this time are floats and integer.
- **void** `DeleteStat(String statName)` Deletes the user's current stat.
- **void** `QueryLeaderboard(String statName, int maxItems = 0)` Try to fetch the leaderboard with the given name. Setting maxItems a number higher than 0 will limit the amount of returned leaderboard values to that amount.
- **void** `QueryLeaderboardSkipToRank(String statName, int rank, int maxItems = 0)` Try to fetch the leaderboard with the given name and skips to rank in leaderboard. Setting maxItems a number higher than 0 will limit the amount of returned leaderboard values to that amount.
- **void** `QueryLeaderboardSkipToSelf(String statName, int maxItems = 0)` Try to fetch the leaderboard with the given name and skips to current user's position. Setting maxItems a number higher than 0 will limit the amount of returned leaderboard values to that amount.
- **void** `QueryLeaderboardForSocialGroup(String statName, String socialGroup, int maxItems = 0)` Try to fetch the leaderboard with the given name and xbox live social group. Setting maxItems a number higher than 0 will limit the amount of returned leaderboard values to that amount.
- **Array** `GetLeaderboardEntries()` Returns the most recently queried leaderboard entries as an array of dictionaries. Each dictionary represents an entry with keys "gamertag", "score", "rank", and "ID". Every value for these keys are strings except for rank, which is an int.
- **Variant** `GetStat(String statName)` Gets a user's stat and returns it as a Godot Variant. Returns -1 if user isn't signed in or if there is an error. Returns "undefined" if the stat hasn't been set.



### Signals

- `signed_out` Called when the user gets signed out. 
- `signed_in` Called when the user successfully signs in.
- `signin_failed` Called when the signin fails. This would be when you have to handly signing in manually if a silent sign in fails, for instance. 
- `leaderboards_ready` Called when a leaderboard query has completed. Passing in a Dictionary with a single key "entries" and value of type Array. The Array is an Array of Dictionary entries. See `GetLeaderbaordEntries()` for info on these entries.
- `stat_user_added` Called when the local user is added to the xbox live stats manager. Don't attempt to query leaderboards or update/get stats for a player before this is called.
- `stat_user_removed` Called when the local user is removed from the xbox live stats manager. Don't attempt to query leaderboards or update/get stats for a player after this is called.
- `player_stats_updated` Called when a SetStat() has completed successfully and player's stats are updated on the leaderboard.


## Exporting

Currently the steps to export are a bit convoluted due to issues with Godot's UWP export, but to do so:

1. Export to UWP from godot with x64 set and in Debug mode.
2. Take the resulting appx and run the following command with 7zip: `7z x YourAPPX.appx -oYourAPPX`.
3. Delete the old appx package.
4. You will see that your DLL has been deleted in the unzipped file. Godot deletes DLL files as it assumed that they will be placed next to the binary like with normal
windows export. Take the DLL from your project and paste it into the same path as it was in your original project, but relative to the `game` folder of the unzipped Appx file.
5. Place an xboxservices.config file in the root of the unzipped appx file. The contents of the config file should be as follows.
```
{
	"TitleId" : "YOUR_TITLE_ID",
	"PrimaryServiceConfigId" : "YOUR_SCID",
	"Sandbox ID" : "YOUR_SANDBOX_ID",
	"XboxLiveCreatorsTitle" : true
}
```
Grab the title, scid, and sandbox ID from your 
partner center panel, where you configure your game. If you aren't familiar with that you should read up on the [Docs](https://docs.microsoft.com/en-us/windows/uwp/xbox-apps/).

6. Use MakeAppx.exe to pack up the contents of your unzipped package with the command `MakeAppx pack /d YourAPPX /p YourAPPX.appx`.
7. Use signtool to sign the package with a trusted .pfx certificate `SignTool sign /fd SHA256 /a /f TrustedCert.pfx /p 111 YourAPPX.appx`.
8. Install your appx after enabling your sandbox and test environment on the partner center and you should be able to sign in. 


### ConsoleCloser?

The ConsoleCloser class is currently used for a workaround to a known issue with the UWP export. It doesn't help with authentication or xbox live integration. 

Currently exported Release UWP application crash on startup no matter what due to a longstanding issue with Godot's UWP export using subsystem/WINDOWS. To work around this
on a release build, you can use EDITBIN on your unzipped appx file with the command `EDITBIN /subsystem:CONSOLE godot.uwp.exe`. This will convert the subsystem with godot
to work off the CONSOLE subsystem, and circumvent the problem. Unfortunately, this also causes the console to open when running the appx and remain open. ConsoleCloser fixes this
by calling FreeConsole on its init(). Just add ConsoleCloser to an autoload scene and the console should close on release builds shortly after booting without problem. 
*This class will be removed as soon as that bug is fixed, but since it has been a problem for a long time, this seemed necessary.*

### Demo

There is a minimal demo included in the demo folder that contains everything you should need to get started on your project, including both authentication and leaderboards. To use the demo you will first need to setup a game on the Xbox Partner Center and enable Xbox Live Services for it. You will also need to add a leaderboard called "HighScore" with an integer format and click the "Test" button on the bottom of the Xbox Services page. Afterwards, just open the game in godot and fill in the export settings using the information from your Partner Center dashboard, including your game's unique name, publisher ID, publisher display name, and GUID. Then export as described above. To test you will need to open up your Windows Developer Settings, enable developer mode and connect to the device portal. In the device portal you can go to the Xbox Live tab and enter in the sandbox ID found on your partner center dashboard in the xbox services section. Once in your sandbox you should be able to to test on PC. To test on Xbox you will need to activate developer mode on your Xbox and follow the instructions dev mode gives you to test your game there. 
