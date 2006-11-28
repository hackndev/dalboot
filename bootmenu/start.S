.text
	b	 _start
dal_start:
	.word 0x0

.globl _start
_start:
	ldr	r0, =0
	bl	main
	b	dal_start
