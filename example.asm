INCADD MACRO ZEN X
	INX ZEN 
	ADD X 
	INX ZEN  
endm

LXI H, 0501H  ; "Get address of first number in H-L pair. Now H-L points to 2501H"  
MOV A, M      ; "Get first operand in accumulator"   
INCADD H M 
MOV M, A      ; "Store result at 2503H"  
HLT           ; "Stop"  
