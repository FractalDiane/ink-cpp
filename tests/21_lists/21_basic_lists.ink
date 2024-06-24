LIST kettleState = cold, boiling, recently_boiled

~ kettleState = cold

The kettle is {kettleState}

- (top)
*	[Turn on kettle]
	The kettle begins to bubble and boil.
	~ kettleState = boiling
+	[Touch the kettle]
	{ kettleState == cold:
		The kettle is cool to the touch.
	- else:
	 	The outside of the kettle is very warm!
	}
- -> top
