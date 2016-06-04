
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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

extern void milli_delay_1(int milli_sec);

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table + NR_TASKS; p++) {
			if (p->ticks > greatest_ticks && !p->sleep) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				p->ticks = p->priority;
			}
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

/*======================================================================*
                           sys_disp_str
 *======================================================================*/
PUBLIC void sys_disp_str(char* str)
{
	disp_str(str);
}

/*======================================================================*
                           sys_disp_color_str
 *======================================================================*/
PUBLIC void sys_disp_color_str(char* str, int color)
{
	disp_color_str(str, color);
}


/*======================================================================*
                           sys_process_sleep
 *======================================================================*/
PUBLIC void sys_process_sleep(int mill_seconds)
{
	p_proc_ready->sleep = mill_seconds * HZ / 1000;
}

/*======================================================================*
                           sys_sem_p
 *======================================================================*/
PUBLIC void sys_sem_p(SEMAPHORE* s)
{
	s->value--;
	if (s->value < 0) {
		s->list[s->head] = p_proc_ready;
		milli_delay_1(10000);
		if (s->head != s->tail) {
			s->head = (s->head + 1) % 10;
		}
	}
	disp_str("p");
}


/*======================================================================*
                           sys_sem_v
 *======================================================================*/
PUBLIC void sys_sem_v(SEMAPHORE* s)
{
	s->value++;
	if (s->value <= 0) {
		s->list[s->tail]->sleep = 0;
//		s->tail = (s->tail + 1) % 10;
	}
	disp_str("v");
}

PUBLIC void P(SEMAPHORE s)
{

}

PUBLIC void V(SEMAPHORE s)
{

}
