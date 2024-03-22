You approach Jim confidently. He doesn't look like he's in the best mood.
"What do you need, cadet?"
-> talk_to_jim ->
Jim leaves to tend to more important matters.
-> END

 === talk_to_jim
 - (opts) 	
	*	[ Ask about the warp nacelles ] 
		-> warp_nacelles ->
	*	[ Ask about the shield generators ] 
		-> shield_generators ->	
	* 	[ Stop talking ]
		->->
 - -> opts 

 = warp_nacelles
	{ shield_generators : ->-> argue }
	"Don't worry about the warp nacelles. They're fine."
	->->

 = shield_generators
	{ warp_nacelles : ->-> argue }
	"Forget about the shield generators. They're good."
	->->
 
 = argue 
 	"What's with all these questions?" Jim demands, suddenly. 
 	"Uh..." you reply. "Just wanted to make sure everything's good, is all."
	"You're far too anxious, cadet," he admonishes you. "Learn to trust people to do their jobs."
 	->->
