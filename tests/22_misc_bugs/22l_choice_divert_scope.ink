//VAR test = false

-> main1.two
=== main1 ===
= one
* (oneone) [1] hi
* (onetwo) [2] hi2
-
-> END

= two
* (oneone) [1] hi3
* (twotwo) [2]
    { test: -> oneone | -> main1.oneone }
-
-> END
