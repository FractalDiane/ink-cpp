-> main
=== function max(a,b) ===
	{ a < b:
		~ return b
	- else:
		~ return a
	}

=== function exp(x, e) ===
	// returns x to the power e where e is an integer
	{ e <= 0:
		~ return 1
	- else:
		~ return x * exp(x, e - 1)
	}

=== main ===
The maximum of 2^5 and 3^3 is {max(exp(2,5), exp(3,3))}.
-> END
