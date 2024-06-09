CONST HALLWAY = 1
CONST OFFICE = 2

VAR player_location = HALLWAY
VAR generals_location = HALLWAY
VAR doctors_location = OFFICE

-> run_player_location

== run_player_location
	{
		- player_location == HALLWAY: -> hallway
		- player_location == OFFICE: -> office
	}

== hallway ==
	<- characters_present(HALLWAY)
	*	[Drawers]	-> examine_drawers
	* 	[Wardrobe] -> examine_wardrobe
	+  [Go to Office] 	-> go_office
	-	-> run_player_location
	
= examine_drawers
	These drawers look suspicious.
	-> run_player_location
	
= examine_wardrobe
    This wardrobe looks conspicuous.
    -> run_player_location

= go_office
    ~ player_location = OFFICE
    -> run_player_location
	
== office ==
    <- characters_present(OFFICE)
    * [Cabinets] -> examine_cabinets
    * [Computer] -> examine_computer
	+ [Go to Hallway] -> go_hallway

= examine_cabinets
	These cabinets look incriminating.
	-> run_player_location
	
= examine_computer
    The computer won't turn on.
    -> run_player_location

= go_hallway
    ~ player_location = HALLWAY
    -> run_player_location



// Here's the thread, which mixes in dialogue for characters you share the room with at the moment.

== characters_present(room)
	{ generals_location == room:
		<- general_conversation
	}
	{ doctors_location == room:
		<- doctor_conversation
	}
	-> DONE

== general_conversation
	*	[Ask the General about the bloodied knife]
		"It's a bad business, I can tell you."
	-	-> run_player_location

== doctor_conversation
	*	[Ask the Doctor about the bloodied knife]
		"There's nothing strange about blood, is there?"
	-	-> run_player_location
