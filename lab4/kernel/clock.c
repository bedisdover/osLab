
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               clock.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

extern void schedule();

/*======================================================================*
                           clock_handler
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	ticks++;
	if (p_proc_ready->sleep == 0) {
		p_proc_ready->ticks--;
	}

	for (int i = 0; i < NR_TASKS; i++) {
		if (proc_table[i].sleep > 0) {
			proc_table[i].sleep--;
		}
	}

	if (k_reenter != 0) {
		return;
	}

	if (p_proc_ready->ticks > 0) {
		return;
	}

	schedule();
}

/*======================================================================*
                              milli_delay
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
        int t = get_ticks();

        while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}


/*======================================================================*
                              milli_delay_1
 *======================================================================*/
PUBLIC void milli_delay_1(int milli_sec)
{
	process_sleep(milli_sec);

	schedule();
//	while (p_proc_ready->sleep) {}
}

/*======================================================================*
                              weakup
 *======================================================================*/
PUBLIC void wakeup(PROCESS* p)
{
	process_wakeup(p);

//	p_proc_ready = p;
}