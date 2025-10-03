
loop:
	loadb r0, [TEST_VAR]
	inc r0
	cmp r0 >= 100
	jt exit
	storeb r0, [TEST_VAR]
	j loop

exit:
	set r0, 5
	push r0
	call func
	pop r0
	halt

func:
	push r5
	set r5, 234

	pop r5
	ret
