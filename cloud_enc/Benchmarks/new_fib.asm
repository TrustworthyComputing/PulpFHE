econst t0, 0
econst t1, 0
econst t2, 0
move t3, 8
secread t4
move t1, t4
secread t4
move t2, t4
move t4, 0
__L1_MAIN__:
cmpl v1, t4, t3
move t5, v1
beq t5, zero, __L2_MAIN__
eadd v1, t1, t2
move t5, v1
move t0, t5
move t1, t2
move t2, t0
add v1, t4, 1
move t4, v1
j __L1_MAIN__
__L2_MAIN__:
answer t0


