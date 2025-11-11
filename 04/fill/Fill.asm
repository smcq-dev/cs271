// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

//// Replace this comment with your code.

(LOOP)
@KBD
D=M
@WHITE
D;JEQ
@BLACK
0;JMP

(WHITE)
@color
M=0
@DRAW
0;JMP

(BLACK)
@color
M=-1
@DRAW
0;JMP

(DRAW)
@SCREEN
D=A
@addr
M=D

(FILL_LOOP)
@color
D=M
@addr
A=M
M=D

@addr
M=M+1
D=M
@KBD
D=A-D

@FILL_LOOP
D;JGT

@LOOP
0;JMP