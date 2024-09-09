-> main
=== function test2(ref y)
~ y = 5

=== function test(ref x)
~ test2(x)


=== main ===
~ temp six = 6
~ test(six)
{six}
-> END 
