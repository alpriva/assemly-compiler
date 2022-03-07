; file with errors for first path to check error handling.

; this is a long comment - results in error -------------------------------------------------------------------------------------------------
; this is a long command - results in error
mov r0,                                                                                           r1

; wrong labels syntax
LAB@EL: cmp r0, r1
1LABEL: cmp r1, r2
LABEL : cmp r2, r3
LABEL:cmp r2, r3
; this line is ok
LABEL: cmp r2, r3

; this line is with label but without command
LABEL: r0        ,       r1
; this line is without command
r0        ,       r1

; this line is with unsupported command
cmd r0, r1

; this line is with unsupported directive
.external EXT1

; ------------------------------------------------
; Checking .data directive errors
; ------------------------------------------------
; this line is data with already defined label
LABEL: .data 1,2,3
; leading comma in 
.data ,1
; two commas between operands
.data 1,,2
; extra comma after command
.data 1,2,
; missing operands
.data
; extra comma
.data 1,
; double instead of int
.data 1.1
; imidiate prefix
.data #1
; illegal char in the integer
.data --1
; illegal char in the integer
.data -1-
; illegal char in the integer
.data 1++
; wrong format 
.data "string"
; long command
.data 1,                                                                                    2
; out of range integer negative
.data -8388609
; out of range integer positive
.data 8388608

; ------------------------------------------------
; Checking .string directive errors
; ------------------------------------------------

; Checking string error directive
; wrong quatation mark
.string 'wrong string'
; empty string followed by extra chars
.string ""wrong string""
; missing quatation mark at the end
.string "wrong string
; missing quatation mark at the start
.string wrong string"
; extra chars
.string "wrong" string"


; ------------------------------------------------
; Checking .extern directive errors
; ------------------------------------------------
; warning on labeling externals
LABEL1: .extern EXT1
; extern with not valid label - first char is non-alpha
.extern 1label
; extern with not valid label - reserved word command
.extern mov
; extern with not valid label - reserved word directive
.extern .string
; extern with not valid label - reserved word register
.extern r1
; extern with not valid label - non-alpha and non-numerical in the middle
.extern lab`el
; extern with not valid label  - non-alpha and non-numerical in the end
.extern label$
; extern with extra chars
.extern label extra chars
; extern without label
.extern
; extern label is already defined in this file.
.extern LABEL


; ------------------------------------------------
; Checking instructions errors
; ------------------------------------------------
; already defined label
LABEL: add r0, r0
; commands without operands but operand is provided
stop r0
rts r1
; commands with 1 operand
; leading comma
clr ,r0
; extra comma
clr r0,
; no operands
clr 
; extra operand
clr r0, r1
; wrong address resolution operand
clr #0
; wrong address resolution operand
clr &LABEL

not ,r0
not r0,
not 
not r0, r1
not #0
not &LABEL

dec ,r0
dec r0,
dec 
dec r0, r1
dec #0
dec &LABEL

inc ,r0
inc r0,
inc 
inc r0, r1
inc #0
inc &LABEL

jmp ,r0
jmp r0,
jmp 
jmp r0, r1
jmp #0
jmp r3

bne ,r0
bne r0,
bne 
bne r0, r1
bne #0
bne r3

jsr ,r0
jsr r0,
jsr 
jsr r0, r1
jsr #0
jsr r3

red ,r0
red r0,
red 
red r0, r1
red #0
red &LABEL

prn ,r0
prn r0,
prn 
prn r0, r1
prn &LABEL

; -------------------------------------------
; command with two operands
; -------------------------------------------
; leading comma
mov ,r0,r1
; two commas in between
mov r0,,r1
; extra comma
mov r0, r1,
; extra operand
mov r0, r1, r2
; missing operand
mov r0
; source operand's address resolution is incorrect
mov &LABEL, r0
; target operand's address resolution is incorrect
mov r0, #1
mov r0, &LABEL

cmp ,r0,r1
cmp r0,,r1
cmp r0, r1,
cmp r0, r1, r2
cmp r0
cmp &LABEL, r0
cmp r0, &LABEL

add ,r0,r1
add r0,,r1
add r0, r1,
add r0, r1, r2
add r0
add &LABEL, r0
add r0, #12
add r0, &LABEL

sub ,r0,r1
sub r0,,r1
sub r0, r1,
sub r0, r1, r2
sub r0
sub &LABEL, r0
sub r0, #12
sub r0, &LABEL

lea ,r0,r1
lea r0,,r1
lea r0, r1,
lea r0, r1, r2
lea r0
lea #0, r0
lea &LABEL, r0
lea r1, r0
lea LABEL, #12
lea LABEL, &LABEL

; warning on label entry
LABEL2: .entry LABEL