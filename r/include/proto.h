
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void TestA();
void TestB();
void TestC();
void TestD();
void TestE();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);

///*信号量*/
//typedef struct semaphore {
//    int value;
//    PROCESS* list[3];
//    int head, tail;
//} SEMAPHORE;

//PUBLIC void P(SEMAPHORE s);
//PUBLIC void V(SEMAPHORE s);


/* 以下是系统调用相关，另有proc.h中的两个系统调用 */

/* proc.c */
PUBLIC  int     sys_get_ticks();        /* sys_call */
PUBLIC  void    sys_disp_str(char *);
PUBLIC  void    sys_disp_color_str(char *, int);
PUBLIC  void    sys_process_sleep(int);

/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
PUBLIC  void    disp_str_1(char *);
PUBLIC  void    disp_color_str_1(char *, int);
PUBLIC  void    process_sleep(int);

