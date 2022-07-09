.data
a: .word  1
b: .word  0
c: .word  0

.text
li $v0, 2
sw $v0, a
lw $v0, a
li $v1, 0
slt $t0, $v0, $v1
beqz $t0, if_1
li $v0, 1
li $v1, 0
mul $t1, $v0, $v1
sw $t1, b
lw $v0, a
lw $v1, b
and $t2, $v0, $v1
sw $t2, c
lw $v0, a
lw $v1, b
or $t3, $v0, $v1
sw $t3, c
j else_2
if_1:
li $v0, 2
li $v1, 2
div $t4, $v0, $v1
sw $t4, c
else_2:
while_1:
lw $v0, a
li $v1, 0
sgt $t5, $v0, $v1
beqz $t5, while_end_1
lw $v0, a
li $v1, 1
sub $t6, $v0, $v1
sw $t6, a
j while_1
while_end_1:
li $v0, 10
syscall
