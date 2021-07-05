extends Node

#This is the manager node. We will instance it on _ready() and keep a reference to it so
#we can query from it and post updates to it as necessary
var xbox_manager

var display_name
var high_score = 0
#If we are signed in or not
var xbox_signedin = false
#If we are in the process of signing in
var xbox_signingin = false
#If the user has been signed in and is ready for stat queries
var xbox_statsready = false
#Dictionary containing leaderboard entries
var leaderboard_data
#Whether the leaderboard query has a result we haven't cached locally yet
var leaderboard_dirty = false

signal xbox_signin
signal xbox_signin_needed
signal leaderboard_ready


func _ready():
	if OS.get_name() == "UWP":
		xbox_manager = Xbox.new()
		add_child(xbox_manager)
		xbox_manager.connect("signed_in", self, "on_xbox_signin")
		xbox_manager.connect("signed_out", self, "on_xbox_signout")
		xbox_manager.connect("signin_failed", self, "on_xbox_failure")
		xbox_manager.connect("stat_user_added", self, "on_xbox_statsready")
		xbox_manager.connect("stat_user_removed", self, "on_xbox_statsremoved")
		xbox_manager.connect("player_stats_updated", self, "on_xbox_statsupdated")
		xbox_manager.connect("leaderboards_ready", self, "on_leaderboard_ready")

#call when we want to sign in, checks against cached xbox_signedin first then signs in
func xbox_signin():
	if !xbox_signedin and xbox_manager != null:
		xbox_signingin = true
		yield(get_tree(), "idle_frame")
		xbox_manager.SignInManually()

#callback when sign in occurs. afterwards you should be safe to get the gamertag
func on_xbox_signin(_args):
	if xbox_manager != null:
		xbox_signedin = true
		xbox_signingin = false
		display_name = xbox_manager.GetGamertag()
		emit_signal("xbox_signin", display_name)

#callback when user signs out. update cached state so we can sign in later
func on_xbox_signout(_args):
	xbox_signedin = false
	xbox_signingin = false
	display_name = ""
	emit_signal("xbox_signin_needed")

#sign in failed, update state so we can attempt again if desired
func on_xbox_failure(_args):
	xbox_signedin = false
	xbox_signingin = false
	display_name = ""
	emit_signal("xbox_signin_needed")

#leaderboards are up to date and ready to display
func on_leaderboard_ready(_args):
	leaderboard_data = _args["entries"]
	leaderboard_dirty = true

#xbox user is signed in and ready for stat queries. don't query leaderboards until this
func on_xbox_statsready(_args):
	xbox_statsready = true
	update_highscore()

#xbox user signed out or otherwise. don't query leaderboards after this
func on_xbox_statsremoved(_args):
	xbox_statsready = false

#callback when user's stats are up to date and in leaderboards
func on_xbox_statsupdated(_args):
	update_highscore()
	query_leaderboard("HighScore")

#call this when you want to query a leaderboard with the string name of the leaderboard
func query_leaderboard(leaderboard):
	if xbox_statsready and xbox_manager != null:
		xbox_manager.QueryLeaderboard(leaderboard, 5)

#call this to update the cached state of the signed in user's stat
func update_highscore():
	if xbox_statsready and xbox_manager != null:
		high_score = xbox_manager.GetStat("HighScore")
		if str(high_score) == "undefined":
			print("HighScore was undefined, defaulting to -1")
			high_score = -1
		if high_score < 0:
			high_score = 0

#call this to set the signed in user's stat on xbox live's backend
func set_highscore(new_score):
	if xbox_statsready and xbox_manager != null:
		high_score = new_score
		xbox_manager.SetStat("HighScore", high_score)
