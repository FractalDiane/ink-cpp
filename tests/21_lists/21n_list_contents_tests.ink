LIST DoctorsInSurgery = (Adams), Bernard, (Cartwright), Denver, Eamonn

{ DoctorsInSurgery: The surgery is open today. | Everyone has gone home. }

{ DoctorsInSurgery == (Adams, Cartwright):
	Dr Adams and Dr Cartwright are having a loud argument in one corner.
}

{ DoctorsInSurgery != (Adams, Bernard):
	At least Adams and Bernard aren't arguing.
}

{ DoctorsInSurgery ? (Adams, Bernard):
	Dr Adams and Dr Bernard are having a hushed argument in one corner.
}

~ DoctorsInSurgery += (Eamonn)

{ DoctorsInSurgery has Eamonn:
	Dr Eamonn is polishing his glasses.
}

{ DoctorsInSurgery hasnt Denver:
	Dr Denver appears to be taking the day off.
}

{DoctorsInSurgery ? ()}
