; This file will be used as a test for the assembler.
; TODO: this should really be much more extensive.

.equ MyValue, 44

	MOV A, MyValue		; Note that we don't need the '#' prefix

.org 3h
	MOV A, #MyValue		; However, the '#' prefix is accepted

.org 7h
	MOV A, 4 + 5
	MOV R0, #4 + #5 << 3	; Here is a nice example of an expression value

	MOV @R1, 0x10 * 2
	MOV @R1, 0x10 / 2
	MOV @R1, 0x10 + 2
	MOV @R1, 0x10 - 2

	MOV @R1, $10

.equ AnotherValue, (0xAA & 0xFF)

	ORL BUS, #AnotherValue
