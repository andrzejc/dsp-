.CODE
xxgetbv	PROC
    mov        ecx, [esp + 4]    
	xgetbv
    ; db	0fh
	; db	01h 
	; db	d0h
    ret
xxgetbv	ENDP
END