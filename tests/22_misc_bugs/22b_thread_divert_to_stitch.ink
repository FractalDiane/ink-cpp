-> gameloop
=== gameloop ===
    Staring down from the heavens you see your followers finishing construction of the last of the great temples, ready to begin the work.
- (top)
    +  [ Regard the temples]
        You regard each of the temples in turn. On each is stacked the rings of stone.
    <- move_post(1, 2)
    <- move_post(2, 2)
    <- move_post(1, 3)
    <- move_post(3, 1)
    <- move_post(3, 3)
    <- move_post(2, 3)
    -> DONE

= move_post(from_post_num, to_post_num)
    +   { from_post_num != to_post_num } [ Move a ring from {from_post_num} to {to_post_num} ]
        You move a ring from {from_post_num} to {to_post_num}
    -> top
