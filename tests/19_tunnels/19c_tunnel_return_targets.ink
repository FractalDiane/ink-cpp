//VAR stamina = 6

-> main
=== fall_down_cliff 
-> hurt(5) -> 
You're still alive! You pick yourself up and walk on.
-> END

=== hurt(x)
	YOU TOOK {x} DAMAGE
	//~ stamina -= x 
	~stamina = stamina - x
	{ stamina <= 0:
		->-> youre_dead
	}
	
	->->

=== youre_dead
Suddenly, there is a white light all around you. Fingers lift an eyepiece from your forehead. 'You lost, buddy. Out of the chair.'
-> END

=== main ===
You're walking down the street, but then the street turns into a cliff and you fall off it.
-> fall_down_cliff
