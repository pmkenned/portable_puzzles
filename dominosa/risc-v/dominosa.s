.equ MAX_DOMINO,    4
.equ NROWS,         4
.equ NCOLS,         5
.equ DIR_NONE,      0
.equ DIR_LEFT,      1
.equ DIR_UP,        2
.equ DIR_DOWN,      3
.equ DIR_RIGHT,     4

.text
_start:
    jal solve
1:
    j 1b

solve:
    addi sp, sp, -232
    sd ra, 224(sp)
    li a0, 0    # row
    li a1, 0    # col
    mv a2, sp   # dominoes
    li a3, 0    # ndominoes
    jal solve_recursive
    lw ra, 224(sp)
    addi sp, sp, 232
    ret

# a0: loc.row
# a1: loc.col
# a2: dominoes
# a3: ndominoes
solve_recursive:
    # stack:
    # 8: 44 ra
    # 4: 40 loc.row
    # 4: 36 loc.col
    # 8: 28 dominoes
    # 4: 24 ndominoes
    # 4: 20 dir
    # 4: 16 domino[1]
    # 4: 12 domino[0]
    # 4:  8 i
    # 4:  4 next_loc.row
    # 4:  0 next_loc.col
    addi sp, sp, -52
    sd ra, 44(sp)

    li t0, DIR_LEFT
    sw t0, 20(sp)
    li t1, DIR_RIGHT
1:  # while
2:  # for
    bgt t0, t1, 3f
    lw a0, 20(sp)
    lw a1, 40(sp)
    lw a2, 36(sp)
    jal can_place_domino
    lw t0, 20(sp)
    li t1, DIR_RIGHT
    bnez a0, 3f     # break
    addi t0, t0, 1
    j 2b
3:  # end of for
    li a0, 0
    bgt t0, t1, 1f

    lw a0, 20(sp) # dir
    lw a1, 40(sp) # row
    lw a2, 36(sp) # col
    jal place_domino

    lw a0, 40(sp) # row
    lw a1, 36(sp) # col
    jal get_domino
    sw a0, 12(sp)
    sw a1, 16(sp)

    lw a2, 28(sp) # dominoes
    lw a3, 24(sp) # ndominoes
    jal domino_already_used
    beqz a0, 2f

    lw a1, 40(sp) # row
    lw a2, 36(sp) # col
    jal remove_domino
    lw t0, 20(sp)
    addi t0, t0, 1
    sw t0, 20(sp)
    j 1b # continue while
2:
    lw t0, 28(sp) # dominoes
    lw t1, 24(sp) # ndominoes
    lw t2, 12(sp) # domino
    slli t1, t1, 3
    add t0, t0, t1
    lw t3, 0(t2)
    sw t3, 0(t0)
    lw t3, 4(t2)
    sw t3, 4(t0)
    srai t1, t1, 3
    addi t1, t1, 1
    sw t1, 24(sp)

    lw a0, 40(sp) # row
    lw a1, 36(sp) # col
    jal find_next_empty_location
    sw a0, 4(sp) # next_loc.row
    sw a1, 0(sp) # next_loc.col
    li t0, NROWS
    bne t0, a0, 2f
    li a0, 1
    j 1f
2:
    lw a2, 28(sp)
    lw a3, 24(sp)
    jal solve_recursive
    beqz a0, 3f
    li a0, 1
    j 1f
3:
    lw a0, 40(sp) # row
    lw a1, 36(sp) # col
    jal remove_domino
    lw t0, 24(sp) # ndominoes
    addi t0, t0, -1
    sw t0, 24(sp)
    lw t1, 20(sp) # dir
    addi t1, t1, 1
    sw t1, 20(sp)
    j 1b
4:  # end of while

1:
    lw ra, 44(sp)
    addi sp, sp, 52
    ret

# a0: dir
# a1: row
# a2: col
can_place_domino:
    mv t1, a0
    li a0, 0

    # t0: tmp
    # t1: dir
    # t2: solution
    # t3: tmp

    # check for edge of board
    bnez a1, 1f
    li t0, DIR_UP
    beq t0, t1, 2f
1:
    bnez a2, 1f
    li t0, DIR_LEFT
    beq t0, t1, 2f
1:
    li t0, NROWS-1
    bne a1, t0, 1f
    li t0, DIR_DOWN
    beq t0, t1, 2f
1:
    li t0, NCOLS-1
    bne a2, t0, 1f
    li t0, DIR_RIGHT
    beq t0, t1, 2f

    # check for neighboring dominoes
    la t2, solution
    mv t0, a1
    mv t3, t0
    slli t0, t0, 2
    add t0, t0, t3
    slli t0, t0, 2
    add t2, t2, t0
    slli a2, a2, 2
    add t2, t2, a2
    # t2: &solution[row][col]

1:
    li t0, DIR_LEFT
    bne t0, t1, 1f
    addi t2, t2, -4
    lw t0, 0(t2)
    bnez t0, 2f
1:
    li t0, DIR_RIGHT
    bne t0, t1, 1f
    addi t2, t2, 8
    lw t0, 0(t2)
    bnez t0, 2f
1:
    li t0, DIR_UP
    bne t0, t1, 1f
    addi t2, t2, -24
    lw t0, 0(t2)
    bnez t0, 2f
1:
    li t0, DIR_DOWN
    bne t0, t1, 1f
    addi t2, t2, 20
    lw t0, 0(t2)
    bnez t0, 2f
1:
    li a0, 1
2:
    ret

# a0: dir
# a1: row
# a2: col
place_domino:

    # t0: &solution[row][col]
    # t1: tmp

    la t0, solution
    slli a1, a1, 2
    mv t1, a1
    slli a1, a1, 2
    add a1, a1, t1
    slli a2, a2, 2
    add t0, t0, a1
    add t0, t0, a2
    sw a0, 0(t0)

    neg a0, a0
    addi a0, a0, 5

    li t1, DIR_LEFT
    beq a0, t1, 1f
    li t1, DIR_DOWN
    beq a0, t1, 2f
    li t1, DIR_UP
    beq a0, t1, 3f

    sw a0, -4(t0)
    j 4f
1:
    sw a0, 4(t0)
    j 4f
2:
    sw a0, -20(t0)
    j 4f
3:
    sw a0, 20(t0)
4:
    ret

# a0: row
# a1: col
get_domino:
    slli a0, a0, 2
    slli a1, a1, 2
    mv t0, a0
    slli a0, a0, 2
    add a0, a0, t0
    add a1, a1, a0
    la t0, board
    la t1, solution
    add t0, t0, a1
    add t1, t1, a1

    lw a0, 0(t1)
    li a1, DIR_RIGHT
    beq a0, a1, 1f
    li a1, DIR_UP
    beq a0, a1, 2f
    li a1, DIR_DOWN
    beq a0, a1, 3f

    lw a0, -4(t0)
    j 4f
1:
    lw a0, 4(t0)
    j 4f
2:
    lw a0, -20(t0)
    j 4f
3:
    lw a0, 20(t0)
4:
    lw a1, 0(t0)

    blt a0, a1, 1f
    xor a0, a0, a1
    xor a1, a1, a0
    xor a0, a0, a1
1:
    ret

# a0: domino.a
# a1: domino.b
# a2: dominoes
# a3: ndominoes
domino_already_used:
    slli a3, a3, 3
    add a3, a2, a3
1:
    bge a2, a3, 3f
    lw t0, 0(a2)
    bne t0, a0, 2f
    lw t0, 4(a2)
    beq t0, a1, 4f
2:
    add a2, a2, 8
    j 1b
3:
    li a0, 0
    ret
4:
    li a0, 1
    ret

# a0: row
# a1: col
remove_domino:
    slli a0, a0, 2
    slli a1, a1, 2
    mv t0, a0
    slli a0, a0, 2
    add a0, a0, t0
    add a1, a1, a0

    la t0, board
    add t0, t0, a1
    la t1, solution
    add t1, t1, a1

    lw a0, 0(t1)
    sw zero, 0(t1)

    li a1, DIR_RIGHT
    beq a0, a1, 1f
    li a1, DIR_UP
    beq a0, a1, 2f
    li a1, DIR_DOWN
    beq a0, a1, 3f

    sw zero, -4(t1)
    ret
1:
    sw zero, 4(t1)
    ret
2:
    sw zero, -20(t1)
    ret
3:
    sw zero, 20(t1)
    ret

# a0: row
# a1: col
find_next_empty_location:
    li t0, NROWS
    li t1, NCOLS
    la t2, solution
    slli a0, a0, 2
    slli a1, a1, 2
1:
    addi a1, a1, 4
    bne a1, t1, 2f
    li a1, 0
    addi a0, a0, 4
    beq a0, t0, 3f
2:
    add t3, t2, a0
    add t3, t3, a1
    lw t3, 0(t3)
    beqz t3, 3f
    j 1b
3:
    srai a0, a0, 2
    srai a1, a1, 2
    ret

# int check_solution();
# void print_board();

# TODO: make these bytes instead of words
.data
board:
.word 2, 2, 1, 3, 0
.word 1, 0, 2, 3, 1
.word 3, 0, 3, 0, 2
.word 3, 2, 0, 1, 1

solution:
.word 0, 0, 0, 0, 0
.word 0, 0, 0, 0, 0
.word 0, 0, 0, 0, 0
.word 0, 0, 0, 0, 0
