; A test driver for all instructions supported by the assembler.

.org 0x0

	add a, r0
	add a, r3
	add a, r7

	add a, @r0
	add a, @r1

	add a, #0x49

	addc a, R1
	addc a, R0
	addc A, r5

	addc a, @r0
	addc a, @r1

	addc a, #0x34
	addc a, #0x9a

	anl a, r2
	anl a, r3
	anl a, r6

	anl a, @r0
	anl a, @r1

	anl a, #0x44
	anl a, #0xfe

label1:
	mov a, .here	; just to show current address in disassembly
	anl bus, #0x16
	anl bus, #0xbd
	anl p1, #0x02
	anl p2, #0xfa

	anld p4, A
	anld p6, A
	anld p7, a

	call label1

	call label2

label2:
	mov a, .here
	clr a
	clr c
	clr f1
	clr f0
	cpl a
	cpl c
	cpl f0
	cpl f1
	da a
	dec a
	dec r0
label3:
	mov a, .here
	dec r3
	dec r6

	dis i
	dis tcnti

	djnz r1, label3
	djnz r4, label4

	en i
	en tcnti
	ent0 clk

	in a, p1
	in a, p2

	inc a

label4:
	mov a, .here

	inc r0
	inc r1
	inc r4
	inc r7

	inc @r0
	inc @r1

	in a, p0	; NOTE: I don't really know the opcode for this instruction
	ins a, bus

	jb0 label1
	jb2 label2
	jb5 label3
	jb7 label4

	jc label1
	jf0 label1
	jf1 label1

	jmp label3

	jmpp @a

	jnc label3

	jni label1
	jnt0 label1
	jnt1 label1

	jnz label1

	jtf label1
	jt0 label1
	jt1 label1
	jz label1

	mov a, #0xda
	mov a, psw
	mov a, r0
	mov a, r5
	mov a, r6
	mov a, @R0
	mov a, @R1

	mov a, t
	mov psw, a
	mov r0, a
	mov r1, a
	mov r7, a
	mov r0, #0xef
	mov r4, #0xFA
	mov r5, #0x34

	mov @r0, a
	mov @r1, a

	mov t, a

	movd A, P4
	movd a, p6
	movd a, p7

	movd P4, a
	movd p6, a
	movd p7, a

	movp a, @a
	movp3 a, @a

	movx a, @r0
	movx a, @r1

	movx @r0, a
	movx @r1, a

	mov a, .here

	nop

	orl a, R1
	orl a, r4
	orl a, r6

	orl a, @r0
	orl a, @r1

	orl a, #0xFA

	orl bus, #0xFA

	orl P1, #0xEE
	orl P2, #0xAA

	orld P4, A

	outl p0, a		; 8021-only!

	outl bus, a

	outl p1, A
	outl p2, A

	ret
	retr

	rl a
	rlc a
	rr a
	rrc a

	sel mb0
	sel mb1

	sel rb0
	sel rb1

	stop tcnt
	strt cnt
	strt t
	swap a

	xch a, r0
	xch a, r4
	xch a, r7
	
	xch a, @r0
	xch a, @r1

	xchd a, @r0
	xchd a, @r1

	xrl a, R0
	xrl a, R3
	xrl a, R4

	xrl A, @r0
	xrl A, @r1

	xrl A, #0xEE
	xrl A, #0x24
