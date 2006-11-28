.text
	b	 _start

@ Magic number so that dalmod.py knows this is definately bootmenu
	.word	0xb007c0de

@ dalmod.py will change this to point to the BigDAL init code
dal_start:
	.word 	0x0

.globl _start
_start:
	bl	main
	b	dal_start
