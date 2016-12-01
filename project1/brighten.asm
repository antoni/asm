global change_brightness

%define RED      1 
%define GREEN    2 
%define BLUE     3

%define MAXVAL   255
%define MINVAL   0

section .text

change_brightness:              ; check which part of the array will be changed
	cmp rcx,RED
	mov r9,0                    ; r9 - pointer to an array of size rows x cols 
	je iterate                  ; with a color to be changed
	cmp rcx,GREEN
	mov r9,1
	je multiply
	cmp rcx,BLUE
	mov r9,2
	jne done                    ; incorrect color => return
multiply:
	imul r9,rsi 
	imul r9,rdx                 
iterate:				        ; start iterating over the chosen color
	add r9,rdi
	mov r10,1 
	imul r10,rsi
	imul r10,rdx                ; r10 - position in the given colors' array
	dec r10                     ; compensate (mov r10,1) to use r10 as an index
choose_operation:               ; choose between add and sub
	mov r11b, r8b               ; r11b - used for temporary storage
	cmp r11b,0                  ; check if brightness change is negative
	jl sub_general
add_general:
	add r11b, byte[r9+r10]      ; add, then check for carry
	jnc next
add_overflow:
	mov r11b, MAXVAL            ; overflow => set value to MAXVAL
sub_general:
	mov r12b,r11b               ; get absolute value without test and/or jump
	mov r13b,r11b               
	sar r13b,7
	xor r13b, r12b
	sub r12b,r13b               ; subtract positive value, then check for carry
	mov r11b,r12b
	jnc next
sub_overflow:
	mov r11b, MINVAL            ; underflow => set value to MINVAL
next:
	mov byte[r9+r10],r11b
	dec r10
	jnz choose_operation
done:
	ret
