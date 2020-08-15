
.CODE 
public is_cpuid_supported

is_cpuid_supported PROC
; OPERATION
; pushfq -> save current flags
;	pushfq -> Push current flags
;		pop -> Get current flags
;	Attempt to insert 1 in ID field
;	pushfq -> Try to modify 1 instead of ID
;		pop -> Check if bit actually changed
; popfq -> Return back to normal state

push rbp
mov rbp, rsp
sub rsp, 128

pushfq

; Get flags registers
pushfq 
pop rax

or rax, rcx

push rax
popfq 

pushfq 
pop rax 

and rax, rcx 
; id MASK
shr rax, 21

popfq

add rsp, 128
pop rbp

ret

is_cpuid_supported ENDP

END