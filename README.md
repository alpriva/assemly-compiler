# Assembly compiler

This project is a compiler 
that translates assembly language (defined especially for
this project) to machine language.
It receives files in assembly language and builds 
files in machine code language.

Example of input file in assembly language :

.entry LIST
.extern W
MAIN: add r3, LIST
LOOP: prn    #48
lea		W,     r6


	inc r6

	mov r3   ,K
	sub r1,r4
	bne 	 END
	cmp	K ,#-6
	bne	&END
	dec W

.entry MAIN
;bdika: heara
	jmp	&LOOP
	add L3 , L3


END:	stop

STR: .string "abcd"
LIST: .data 6,  -9
.data -100
K: .data 31

.extern L3

For this file the program will generate 3 output files:

1: Object file. First line represents two two decimal numbers:
  Length of instruction image and length of data image.
  Other lines contains memory image. In each line there are two fields :
  the address of the word(decimal) and the content of the word(hexadecimal)
     25 9
0000100 0b680c
0000101 000412
0000102 340004
0000103 000184
0000104 111e04
0000105 000001
0000106 141e1c
0000107 036804
0000108 00042a
0000109 0b3c14
0000110 240814
0000111 0003e2
0000112 050004
0000113 00042a
0000114 ffffd4
0000115 241014
0000116 00004c
0000117 140824
0000118 000001
0000119 24100c
0000120 ffff7c
0000121 09080c
0000122 000001
0000123 000001
0000124 3c0004
0000125 000061
0000126 000062
0000127 000063
0000128 000064
0000129 000000
0000130 000006
0000131 fffff7
0000132 ffff9c
0000133 00001f

2:Entries file. Contains labels that characterized
 as entries with their addresses.

MAIN 0000100
LIST 0000130

3: Externals file. Contains labels that characterized
  as externals with their addresses

W 0000105
W 0000118
L3 0000122
L3 0000123

In case of errors in input file or other errors, the program
will generate output file with the list of errors.
