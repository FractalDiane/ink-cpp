VAR stamina = 10

-> main

=== function harm(x) ===
	{ stamina < x:
		~ stamina = 0
	- else:
		~ stamina = stamina - x
	}

=== main ===
You have {stamina} HP.
A stray pebble flies in and hits you on the head. It deals no damage, but you are so startled that you fall over and land on your face.
You took 2 damage. {harm(2)}
You now have {stamina} HP.
-> END
