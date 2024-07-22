LIST Discs = one, two, three, four, five, six, seven
VAR post1 = ()
VAR post2 = ()
VAR post3 = ()

~ post1 = LIST_ALL(Discs)

-> gameloop

=== function postToPlace(postNum)
    { postNum:
        - 1: first
        - 2: second
        - 3: third
    }

=== function name(postNum)
    the {postToPlace(postNum)} temple

=== function Name(postNum)
    The {postToPlace(postNum)} temple

== function getListForTower(towerNum)
    { towerNum:
        - 1:    ~ return post1
        - 2:    ~ return post2
        - 3:    ~ return post3
    }

=== function describe_pillar(listNum) ==
    ~ temp list = getListForTower(listNum)
    {
    - LIST_COUNT(list) == 0:
        {Name(listNum)} is empty.
    - LIST_COUNT(list) == 1:
        The {list} ring lies on {name(listNum)}.
    - else:
        On {name(listNum)}, are the discs numbered {list}.
    }


=== gameloop ===
This is a test. {describe_pillar(1)}
//~ temp list = getListForTower(1)
//On {name(1)}, are the discs numbered {list}.
-> END

// when we hit an interpolate, don't stop until there's more content and a newline???????
