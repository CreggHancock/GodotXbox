extends Control

export(PackedScene) var world_scene

export(NodePath) var player_name_path
export(NodePath) var player_animation_path

onready var player_name = get_node(player_name_path)
onready var player_animation = get_node(player_animation_path)

func _ready():
	_set_player_name("Press X To Sign In")
	if OS.get_name() == "UWP":
		Global.connect("xbox_signin", self, "on_xbox_signin")
		Global.connect("xbox_signin_needed", self, "on_xbox_signout")
		if Global.display_name != "":
			if Global.xbox_signedin:
				_set_player_name(Global.display_name)
			elif !Global.xbox_signingin:
				on_xbox_signout()
			else:
				_set_player_name("")
		else:
			_set_player_name(Global.display_name)
			
func on_xbox_signin(gamertag):
	_set_player_name(gamertag)
	
func on_xbox_signout():
	player_animation.stop(true)
	_set_player_name("Press X To Sign In")


func _input(event):
	if event.is_action_pressed("ui_accept"):
		get_tree().change_scene_to(world_scene)
	
	if OS.get_name() == "UWP":
		if event.is_action_pressed("signin") and !Global.xbox_signingin and !Global.xbox_signedin:
			player_animation.play("signingin")
			Global.xbox_signin()


func _set_player_name(display_name):
	player_animation.stop(true)
	player_name.text = display_name
