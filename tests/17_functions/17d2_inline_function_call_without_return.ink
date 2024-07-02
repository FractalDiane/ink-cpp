-> main
=== function describe_health(x) ===
{
- x == 100:
	spritely
- x > 75:
	chipper
- x > 45:
	somewhat flagging
- else:
	despondent
}

=== main ===
Monsieur Fogg was looking {describe_health(health)}.
