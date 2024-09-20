-> main
=== main ===
- (test) hi
-> test2
* One
* Two
    ** (test2) hi2
        hi3
    --
- (after)
-> nested
* One
	here's a bunch of text
	-- (nested) and then a gather point
* Two
-
-> knot2.nested

== knot1
* One
	here's some text
	** One
		here's some more text
	--

	** Two
		here's some more text
		--- (nested) and then in a knot
	--
	hello
* Two
-

-> END

== knot2
* One
	here's some text2
	** One
		here's some more text2
	--

	** Two
		here's some more text2
		--- (nested) and then in a knot2
	--
	hello2
* Two
-

-> END
