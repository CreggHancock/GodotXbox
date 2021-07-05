extends VBoxContainer

export(NodePath) var title_path
export(Array) var leaderboard_entry_paths

onready var title_label = get_node(title_path)

func set_leaderboard():
	Global.leaderboard_dirty = false
	var entries = Global.leaderboard_data
	if entries == null:
		print("Entries were null, trying to query leaderboard again")
		Global.query_leaderboard("HighScore")
		yield(get_tree().create_timer(3), "timeout")
		if !Global.leaderboard_dirty:
			print("Query failed")
			return
	var ind = 0
	for path in leaderboard_entry_paths:
		var entry_label = get_node(path)
		if entries.size() > ind:
			entry_label.show()
			var entry = entries[ind]
			entry_label.text = str(entry["rank"]) + ". " + entry["gamerTag"] + ": " + entry["score"]
		else:
			entry_label.hide()
			if ind == 0:
				return
		ind += 1
	title_label.show()

