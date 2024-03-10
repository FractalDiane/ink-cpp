-> main
=== function describe_health(x) ===
{
- x == 100:
	~ return "spritely"
- x > 75:
	~ return "chipper"
- x > 45:
	~ return "somewhat flagging"
- else:
	~ return "despondent"
}

=== main ===
Monsieur Fogg was looking {describe_health(health)}.
