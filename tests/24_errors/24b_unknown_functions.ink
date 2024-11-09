INCLUDE ../functions.ink

{ 
  - get_flag("angel_contract_pen_supplied"):
  - true:
    -> task_complete
  - else:
    -> repeat_chatter
}

=== repeat_chatter ===
#angel #happy
#none
(he's excitedly talking with the mice)
(they seem overjoyed to be talking to him)
#angel #neutral
-> END

=== task_complete ===
#player
got the pen!
and the contract, too!

#mouse
perfect!
thanks {player_name()}!
now,
let us conduct the micening of this contract

#angel #sweat
...micening?

#mouse
micening!
this contract is toast!

#wait 0.5

#fadeout 2 1.0 1.0 1.0
#wait 3
aaand bam!
there we go!
contract broken!

TODO: get rid of pen + contract, add sound effect

#fadein 2 1.0 1.0 1.0
#wait 3

#angel #
it's... as easy as that?

#mouse
yep!

#angel #puppy_eyes
wow...
this feels
kinda surreal

#wait 1

#lookat angel 1

#angel #happy
{player_name()}
thank you so much for your help with this

#player
sure thing, angel!

#wait 0.5

#angel #pout
i understand that doing this is gonna have some...
monetary disadvantages
but i just,
really needed this
i hope this can give me a bit of a break

#player
yeah, absolutely

#angel #neutral
besides, i think working with these
mice?
mouses?
meese?
...is gonna be a much better deal #happy

#mouse
you can count on us! squeak!

#wait 1

#angel #neutral
you know
i think i need to thank you a bit more
why don't you take this?

#none
(he hands you a small painting)
(it's a similar style to the mural he painted)

#angel
i made this in my spare time a while back
i've gotten a lot better since then, but
i'm still pretty proud of it #happy
#wait 0.5
sometimes it's time to move on, though #neutral
so i'd like you to have it

#player
thank you, angel!

#angel #happy
sure thing!
#wait 0.5
well, #neutral
i should probably work out the details with these guys
of our lil deal

#player
alright, i'll leave you to it!

~ set_flag("angel_quest_stage", 4)
~ set_flag("angel_quest_complete", true)
~ pass_time()

-> END
