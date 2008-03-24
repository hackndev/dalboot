#include "main.h"
#include "pxa-regs.h"

#define DEBOUNCE_INTERVAL	32 /* ms */

keypad_matrix *kp;

void init_keypad(keypad_matrix *keypad)
{
	int i;

	kp = keypad;

	for (i=0; kp->out_gpios[i] >= 0; i++) {
		pxa_gpio_mode(kp->out_gpios[i]);
	}

	for (i=0; kp->in_gpios[i] >= 0; i++) {
		pxa_gpio_mode(kp->in_gpios[i]);
	}

	if (kp->pxa27x) {
		KPC = KPC_ME | ((kp->cols-1)&0xf) << 23 |
				((kp->rows-1)&0xf) << 26 |
				KPC_MS0 | KPC_MS1 | KPC_MS2 | KPC_MS3 |
				KPC_MS4 | KPC_MS5 | KPC_MS6 | KPC_MS7;
		KPKDI = DEBOUNCE_INTERVAL;
		CKEN |= CKEN19_KEYPAD;
	}
}

int read_keypad()
{	
	int col, row;
	if (!kp) return 0;

	if (kp->pxa27x) {
		KPC |= KPC_AS; /* initiate scan */
		while (KPAS & KPAS_SO) switch_led(); /* wait for completion */

		if (KPAS & (1<<26))  { /* something pressed? */
			col = KPAS & 0xf;
			row = (KPAS >> 4) & 0xf;
			return kp->keymap[row][col];
		}
	} else {
		for (col=0; kp->out_gpios[col] >= 0; col++) {
			SET_GPIO(kp->out_gpios[col], 1);
			for (row=0; kp->in_gpios[row] >= 0; row++) {
				if (GET_GPIO(kp->in_gpios[col])) {
					SET_GPIO(kp->out_gpios[col], 0);
					return kp->keymap[row][col];
				}
			}
			SET_GPIO(kp->out_gpios[col], 0);
		}
	}
	return 0;
}

int getchar()
{
	int c=0;
	/* make sure key is released first */
	while (read_keypad());
	/* wait for key press */
	while (!c) c = read_keypad();
	return c;
}

void wait_input()
{
        int k;
        while (1) {
                k = getchar();
//              if (k) putchar(k);
                switch (k) {
                case 'h': return;
                }
        }
}

