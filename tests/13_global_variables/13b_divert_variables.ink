VAR 	current_epilogue = -> everybody_dies

Your Kingdom is in dire straits.
-> continue_or_quit

=== continue_or_quit ===
Give up now, or keep trying to save your Kingdom?
+  [Keep trying!] 	-> more_hopeless_introspection
+  [Give up] 		-> current_epilogue

=== more_hopeless_introspection ===
It's not working. Your Kingdom is still in dire straits.
-> continue_or_quit

=== everybody_dies ===
Everybody in your Kingdom died.
-> END
