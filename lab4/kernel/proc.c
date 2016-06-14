
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
extern void wakeup(PROCESS*);
extern void openIRQ();
extern void closeIRQ();
/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

//	disp_str(p_proc_ready->p_name);
//	disp_int(p_proc_ready->ticks);
//	disp_int(proc_table[1].ticks);
//	disp_color_int(proc_table[1].sleep, 0x06);
//	disp_color_int(proc_table[2].ticks, 0x09);
//	disp_color_int(proc_table[2].sleep, 0x08);

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table + NR_TASKS; p++) {
			if (p->ticks > greatest_ticks && p->sleep >= 0) {
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
                           sys_process_wakeup
 *======================================================================*/
PUBLIC void sys_process_wakeup(PROCESS* p) {
	p->sleep = 0;
}


/*======================================================================*
                           sys_sem_p
 *======================================================================*/
PUBLIC void sys_sem_p(SEMAPHORE* s)
{
//	closeIRQ();
	s->value--;
	if (s->value < 0) {
		s->list[s->head] = p_proc_ready;
		s->head = (s->head + 1) % QUEUE_LENGTH;
		milli_delay_1(-10);
	}
//	openIRQ();                     /* 让8259A可以接收时钟中断 */
//	disp_str("p3");
}


/*======================================================================*
                           sys_sem_v
 *======================================================================*/
PUBLIC void sys_sem_v(SEMAPHORE* s)
{
//	closeIRQ();
	s->value++;
	if (s->value <= 0) {
		PROCESS* p = s->list[s->tail];
		s->tail = (s->tail + 1) % QUEUE_LENGTH;
		wakeup(p);
//		disp_str(p->p_name);
	}
//	openIRQ();                     /* 让8259A可以接收时钟中断 */
}
