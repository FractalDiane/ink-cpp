LIST OnOff = on, off
LIST HotCold = cold, warm, hot

VAR kettleState = (off, cold) // we need brackets because it's a proper, multi-valued list now

-> main

=== function turnOnKettle() ===
{ kettleState ? hot:
	You turn on the kettle, but it immediately flips off again.
- else:
	The water in the kettle begins to heat up.
	~ kettleState -= off
	~ kettleState += on
	// note we avoid "=" as it'll remove all existing states
}

=== function can_make_tea() ===
	~ return kettleState ? (hot, off)

=== function changeStateTo(ref stateVariable, stateToReach)
	// remove all states of this type
	~ stateVariable -= LIST_ALL(stateToReach)
	// put back the state we want
	~ stateVariable += stateToReach

=== main
{kettleState}
~ changeStateTo(kettleState, hot)
{kettleState}
-> END
