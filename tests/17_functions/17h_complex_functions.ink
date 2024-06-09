-> main
=== function say_yes_to_everything ===
	hello
	there
	this
	is
	a
	test
	~ return true
	
=== function yes2 ===
hi
~ return true

=== function lerp(a, b, k) ===
	~ return ((b - a) * k) + a

=== function test ===
hello
there
~ return 2

=== main ===
~ temp x = lerp(2, 8, 0.3)
~ temp y = test()
x = {x}
y = {y}
{test() + test()}
* {say_yes_to_everything()} Yes.
* {yes2()} No.
- -> END
