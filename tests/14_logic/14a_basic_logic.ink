VAR knows_about_wager = false
VAR x = 5
VAR y = 2
VAR c = 6
VAR p1 = 0
VAR p2 = 0

~ knows_about_wager = true
~ x = (x * x) - (y * y) + c
~ y = 2 * x * y
~ p1 = POW(3, 2)
~ p2 = POW(16, 0.5)

knows_about_wager = {knows_about_wager}
x = {x}
y = {y}
p1 = {p1}
p2 = {p2}
