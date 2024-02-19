// VAR door_open = false

Monsieur Fogg and I stared at each other for several long moments.
{ door_open:
	* 	I strode out of the compartment[] and I fancied I heard my master quietly tutting to himself. 			-> go_outside
- else:
	*	I asked permission to leave[] and Monsieur Fogg looked surprised. 	-> open_door
	* 	I stood and went to open the door[]. Monsieur Fogg seemed untroubled by this small rebellion. -> open_door
}

=== go_outside ===
I went outside.
-> END

=== open_door ===
I opened the door and left.
-> END
