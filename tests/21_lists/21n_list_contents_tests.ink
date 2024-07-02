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

LIST Facts = (Fogg_is_fairly_odd), 	first_name_phileas, (Fogg_is_English)

{Facts ? Fogg_is_fairly_odd:I smiled politely.|I frowned. Was he a lunatic?}
'{Facts ? first_name_phileas:Phileas|Monsieur}, really!' I cried.

{ Facts ? (Fogg_is_English, Fogg_is_fairly_odd):
	<> 'I know Englishmen are strange, but this is *incredible*!'
}
