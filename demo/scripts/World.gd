extends Node2D

export(NodePath) var timer_path
export(NodePath) var character_path
export(NodePath) var score_label_path
export(NodePath) var character_anim_path
export(NodePath) var end_popup_path
export(NodePath) var retry_button_path
export(NodePath) var menu_button_path
export(NodePath) var high_score_path
export(NodePath) var leaderboard_path
export(float) var max_time
export(float) var countdown_speed

onready var countdown_timer = get_node(timer_path)
onready var character = get_node(character_path)
onready var score_label = get_node(score_label_path)
onready var character_anim = get_node(character_anim_path)
onready var end_game_popup = get_node(end_popup_path)
onready var retry_button = get_node(retry_button_path)
onready var menu_button = get_node(menu_button_path)
onready var high_score = get_node(high_score_path)
onready var leaderboard = get_node(leaderboard_path)

var score = 0
var game_over = false

const HIGH_SCORE_FORMAT = "High Score: "
const SCORE_FORMAT = "Score: "


func _ready():
	Global.query_leaderboard("HighScore")
	countdown_timer.max_value = max_time
	countdown_timer.value = max_time
	randomize()


func _process(delta):
	if countdown_timer.value > 0:
		countdown_timer.value -= delta * countdown_speed
		if countdown_timer.value <= 0 and !game_over:
			end_game_popup.popup_centered()
			retry_button.grab_focus()
			var best_score = score if score > Global.high_score else Global.high_score
			high_score.text = HIGH_SCORE_FORMAT + str(best_score)
			if best_score > Global.high_score:
				Global.set_highscore(best_score)
			game_over = true
	elif countdown_timer.value <= 0 and Global.leaderboard_dirty:
		leaderboard.set_leaderboard()


func _input(event):
	if countdown_timer.value > 0:
		if event.is_action_pressed("ui_accept"):
			score += 1
			score_label.text = SCORE_FORMAT + str(score)
			character_anim.play("pressed")
			var pressed_color = Color(randf(), randf(), randf(), 1)
			character.self_modulate = pressed_color


func _on_Retry_pressed():
	get_tree().reload_current_scene()


func _on_Menu_pressed():
	get_tree().change_scene_to(load("res://scenes//Title.tscn"))
