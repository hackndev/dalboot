.text
.globl _start
_start:
	b	 _copier

@ Magic number so that dalmod.py knows this is definately bootmenu
	.word	0xb007c0de

@ dalmod.py will change this to point to the BigDAL init code
dal_start:
	.word 	0x0

_copier:
@ Figure out where _start was loaded in RAM and pass it to copier
	mov	r0, pc
	sub	r0, #(_copier-_start+8)
	bl	relocate_bootmenu

@ Call main()
	mov	lr, pc
	mov	pc, r0

@ Let DAL continue booting PalmOS
	b	dal_start


.globl get_cpu_id
@ This ID can be used to determine the manufacturer and model of the CPU.
get_cpu_id:
	mrc p15, 0, r0, c0, c0, 0
	mov	pc, lr

