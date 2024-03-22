-> plains
=== plains ===
= night_time
	The dark grass is soft under your feet.
	+	[Sleep]
		-> sleep_here -> wake_here -> day_time
= day_time
	It is time to move on.
	-> END

=== wake_here ===
	You wake as the sun rises.
	+	[Eat something]
		-> eat_something ->
	+	[Make a move]
	-	->->

=== sleep_here ===
	You lie down and try to close your eyes.
	-> monster_attacks ->
	Then it is time to sleep.
	-> dream ->
	->->

=== eat_something ===
You pull out a submarine sandwich from your coat pocket and take a bite.
How exactly did it fit in there? You don't know nor care.
->->

=== monster_attacks ===
A goblin attacks you!
Irritated, you yell "GO AWAY!" at the top of your lungs.
The goblin dejectedly slinks away, defeated.
->->

=== dream ===
You have a dream about snakes. Is this a shameless callback to test case 15F? Absolutely.
->->
