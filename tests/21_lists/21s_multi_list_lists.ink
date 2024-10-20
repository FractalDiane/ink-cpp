LIST Characters = Alfred, Batman, Robin
LIST Props = champagne_glass, newspaper

VAR BallroomContents = (Alfred, Batman, newspaper)
VAR HallwayContents = (Robin, champagne_glass)

-> main

=== function describe_room(roomState)
	{ roomState ? Alfred: Alfred is here, standing quietly in a corner. } { roomState ? Batman: Batman's presence dominates all. } { roomState ? Robin: Robin is all but forgotten. }
	<> { roomState ? champagne_glass: A champagne glass lies discarded on the floor. } { roomState ? newspaper: On one table, a headline blares out WHO IS THE BATMAN? AND *WHO* IS HIS BARELY-REMEMBERED ASSISTANT? }

=== main
ballroom:
{ describe_room(BallroomContents) }
hallway:
{ describe_room(HallwayContents) }

{ BallroomContents ? (Batman, Alfred): To reiterate, Batman and Alfred are in the ballroom. }

-> END
