-> main
=== function do_nothing_2(x)
~ x += 5

=== function do_nothing(x)
~ x += 3
~ do_nothing_2(x)

=== main
~ temp x = 20
~ do_nothing(x)
{x}
-> END
