X: .string "ab cd	ef " 
mov #-23, X 
cmp Y, r6 
Y: .data 105, -4, -1, 5 
clr Y 
add S, r1 
.extern S 
dec Y 
jmp &Y 
prn #7 
sub S, Y 
prn X 
stop 
;end of code
 
