LIST Discs = one, two, three, four, five, six, seven
VAR post1 = ()
VAR post2 = ()
VAR post3 = ()

~ post1 = LIST_ALL(Discs)

-> gameloop

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
