//VAR fear = 0
//VAR visited_snakes = false
//VAR visited_poland = false

-> dream
=== dream ===
	{
		- visited_snakes && not dream_about_snakes:
			~ fear++
			-> dream_about_snakes

		- visited_poland && not dream_about_polish_beer:
			~ fear--
			-> dream_about_polish_beer

		- else:
			// breakfast-based dreams have no effect
			-> dream_about_marmalade
	}

=== dream_about_snakes ===
You had a dream about snakes. Thousands of them. Oh dear.
-> END

=== dream_about_polish_beer ===
You had a dream about Polish beer. Unfortunately, you don't drink.
-> END

=== dream_about_marmalade ===
You had a dream about marmalade. You prefer strawberry jam, but it's not bad.
-> END
