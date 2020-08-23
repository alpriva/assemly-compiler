; file with errors for second path to check error handling.

; --------------------------------------------------
; Entry validation
; --------------------------------------------------
; 1. entry with not valid label - first char is non-alpha
.entry 1label

; 2. entry with not valid label - reserved word command
.entry mov

; 3. entry with not valid label - reserved word directive
.entry .string

; 4. entry with not valid label - reserved word register
.entry r1

; 5. entry with not valid label - non-alpha and non-numerical in the middle
.entry lab`el

; 6. entry with not valid label  - non-alpha and non-numerical in the end
.entry label$

; 7. entry with extra chars
.entry label extra chars

; 8. entry without label
.entry

; 9. entry no such label
.entry LABEL1

; 10. no externals are allowed to be .entry
; define extern var EXT1
.extern EXT1
.entry EXT1

; --------------------------------------------------
; Other command completion validation
; --------------------------------------------------

; 11. direct source label doesn't exist
LABEL2: mov LABEL3, STR

; 12. direct target label doesn't exists
cmp STR, LABEL3

; this line is ok.
STR: .string "example"

; 13. relative target label doesn't exists
jmp &LABEL3

; 14. external ref is not allowed with relative
bne &EXT1
