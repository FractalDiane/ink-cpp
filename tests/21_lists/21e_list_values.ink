LIST volumeLevel = off, quiet, medium, loud, deafening
VAR lecturersVolume = quiet
VAR murmurersVolume = quiet

The lecturer is {lecturersVolume}.
There is some {murmurersVolume} murmuring in the hall.
- (top)
{ lecturersVolume < deafening:
	~ lecturersVolume++
	The lecturer raises their voice. They are now {lecturersVolume}.

	{ lecturersVolume > murmurersVolume:
		~ murmurersVolume++
		The murmuring gets louder. It's now {murmurersVolume}.
	}
  - else:
	The lecturer yells at the top of their lungs!
	They give themself a sore throat and leave the room.
	-> END
}
-> top
