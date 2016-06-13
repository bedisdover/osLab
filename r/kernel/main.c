
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
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

extern void enable_irq(int irq);
extern char* strcpy(char* p_dst, char* p_src);
extern void milli_delay(int milli_sec);
extern void milli_delay_1(int milli_sec);
extern void disp_color_int(int input, int color);

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid
		p_proc->sleep = 0;
		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;

		proc_table[i].sleep = 0;
	}

	// 清空屏幕
	disp_pos = 0;
	for (i = 0; i < 80 * 25; i++) {
		disp_str(" ");
	}
	disp_pos = 0;

	proc_table[0].ticks = proc_table[0].priority =  1;
	proc_table[1].ticks = proc_table[1].priority =  1;
	proc_table[2].ticks = proc_table[2].priority =  1;
	proc_table[3].ticks = proc_table[3].priority =  1;
	proc_table[4].ticks = proc_table[4].priority =  1;

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	waiting			= 0;
	number			= 0;

	customers.value = 0;
	customers.head  = 0;
	customers.tail  = 0;

	barbers.value 	= 0;
	barbers.head  	= 0;
	barbers.tail  	= 0;

	mutex.value 	= 1;
	mutex.head  	= 0;
	mutex.tail  	= 0;

	//	semaphore.list[0] = p_proc_ready;

	/* 初始化 8253 PIT */
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
	enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

	restart();

	while(1){}
}

void come(int number)
{
	disp_color_str_1("customer ", 0x05);
	disp_color_int(number, 0x05);
	disp_color_str_1(" come\n", 0x05);
	milli_delay(1000);
}

void getHaircut(int number)
{
	disp_color_str_1("customer ", 0x06);
	disp_color_int(number, 0x06);
	disp_color_str_1(" get haircut\n", 0x06);
	milli_delay(2000);
}

void leave(int number)
{
	disp_color_str_1("customer ", 0x06);
	disp_color_int(number, 0x06);
	disp_color_str_1(" leave\n", 0x06);
	milli_delay(1000);
}

void full()
{
	disp_color_str_1("full, customer leave\n", 0x09);
	milli_delay(1000);
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	int i = 0;
	while (1) {
		milli_delay(10);
	}
}

/*======================================================================*
                               TestB（理发师进程）
 *======================================================================*/
void TestB()
{
	int i = 0x1000;
	while(1){
//		milli_delay(1000);
		sem_p(&customers);				/*判断是否有顾客，若无顾客，理发师睡眠*/
//		milli_delay(1000);
		sem_p(&mutex);					/*若有顾客，进入临界区*/
		waiting--;						/*等待顾客数减1*/
		sem_v(&barbers);				/*理发师准备为顾客理发*/
		milli_delay(2000);				/*理发师正在理发（非临界区）*/
		disp_color_str_1("barber cut hair\n", 0x04);
		sem_v(&mutex);					/*退出临界区*/
	}
}

/*======================================================================*
                               TestC（顾客进程）
 *======================================================================*/
void TestC()
{
	while(1) {
//		sem_v(&barbers);
//		sem_v(&customers);
		sem_p(&mutex);					/*进入临界区*/
		if (waiting < CHAIRS) {			/*判断是否有空椅子*/
			waiting++;					/*等待顾客加1*/
			number++;					/*顾客编号加1*/
			come(number);
			sem_v(&customers);			/*唤醒理发师*/
			sem_v(&mutex);				/*退出临界区*/
//			disp_str("1");
			sem_p(&barbers);			/*理发师忙，顾客坐着等待*/
//			disp_str("2");
			sem_p(&mutex);
			getHaircut(number);
			leave(number);
			sem_v(&mutex);
		} else {
			sem_v(&mutex);				/*人满了，顾客离开*/
			sem_p(&mutex);
			full();
			sem_v(&mutex);
		}
	}
}

/*======================================================================*
                               TestD
 *======================================================================*/
void TestD()
{
	while(1) {
//		sem_v(&barbers);
//		sem_v(&customers);
		sem_p(&mutex);					/*进入临界区*/
		if (waiting < CHAIRS) {			/*判断是否有空椅子*/
			waiting++;					/*等待顾客加1*/
			number++;					/*顾客编号加1*/
			come(number);
			sem_v(&customers);			/*唤醒理发师*/
			sem_v(&mutex);				/*退出临界区*/
//			disp_str("1");
			sem_p(&barbers);			/*理发师忙，顾客坐着等待*/
//			disp_str("2");
			sem_p(&mutex);
			getHaircut(number);
			leave(number);
			sem_v(&mutex);
		} else {
			sem_v(&mutex);				/*人满了，顾客离开*/
			sem_p(&mutex);
			full();
			sem_v(&mutex);
		}
	}
}

/*======================================================================*
                               TestE
 *======================================================================*/
void TestE()
{
	while(1) {
//		sem_v(&barbers);
//		sem_v(&customers);
		sem_p(&mutex);					/*进入临界区*/
		if (waiting < CHAIRS) {			/*判断是否有空椅子*/
			waiting++;					/*等待顾客加1*/
			number++;					/*顾客编号加1*/
			come(number);
			sem_v(&customers);			/*唤醒理发师*/
			sem_v(&mutex);				/*退出临界区*/
//			disp_str("1");
			sem_p(&barbers);			/*理发师忙，顾客坐着等待*/
//			disp_str("2");
			sem_p(&mutex);
			getHaircut(number);
			leave(number);
			sem_v(&mutex);
		} else {
			sem_v(&mutex);				/*人满了，顾客离开*/
			sem_p(&mutex);
			full();
			sem_v(&mutex);
		}
	}
}