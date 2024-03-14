VAR health = 10
VAR foggs_health = 14

-> main 
=== function alter(ref x, k) ===
	~ x = x + k

=== main ===
You have {health} HP.
Monsieur Fogg has {foggs_health} HP.
*	I ate a biscuit[] and felt refreshed. {alter(health, 2)}
* 	I gave a biscuit to Monsieur Fogg[] and he wolfed it down most undecorously. {alter(foggs_health, 1)}
-	<> Then we continued on our way.

You have {health} HP.
Monsieur Fogg has {foggs_health} HP.
