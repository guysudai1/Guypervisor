.CODE 
public GetEsSelector
public GetCsSelector
public GetSsSelector
public GetDsSelector
public GetFsSelector
public GetGsSelector
public GetLdtrSelector
public GetTrSelector
public __read_rsp
public __read_esp

GetEsSelector PROC
    ; Returns es selector register
    mov ax, es
    ret
GetEsSelector ENDP

GetCsSelector PROC
    ; Returns cs selector register
    mov ax, cs
    ret
GetCsSelector ENDP

GetSsSelector PROC
    ; Returns ss selector register
    mov ax, ss
    ret
GetSsSelector ENDP

GetDsSelector PROC
    ; Returns ds selector register
    mov ax, ds
    ret
GetDsSelector ENDP

GetFsSelector PROC
    ; Returns fs selector register
    mov ax, fs
    ret
GetFsSelector ENDP

GetGsSelector PROC
    ; Returns gs selector register
    mov ax, gs
    ret
GetGsSelector ENDP

GetLdtrSelector PROC
    ; Returns ldtr selector register
    sldt ax
    ret
GetLdtrSelector ENDP

GetTrSelector PROC
    ; Returns tr selector register
    str ax
    ret
GetTrSelector ENDP

__lar PROC
    ; Loads access rights into descriptor
    lar rax, rcx
    ret
__lar ENDP

__read_rsp PROC
    mov rax, rsp
    add rax, 8
    ret
__read_rsp ENDP
    
__read_esp PROC
    mov eax, esp
    add eax, 4
    ret
__read_esp ENDP

END