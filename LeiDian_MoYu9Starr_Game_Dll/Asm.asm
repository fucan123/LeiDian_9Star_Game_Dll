.CODE

Asm_Nd PROC
    mov eax,edx ;功能号
    xor r9d,r9d  
    xor r8d,r8d  
    mov edx,11h ;禁止反调试
    mov r10,rcx
	
	syscall

	ret
Asm_Nd ENDP

END