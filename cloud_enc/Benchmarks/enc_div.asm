econst t0, 0
econst t1, 0
econst t2, 0
secread t3
move t0, t3
secread t3
move t1, t3
ediv v1, t0, t1
move t0, v1
move t2, t0
answer t2
