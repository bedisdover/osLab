/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"
#include "../include/tty.h"
#include "../include/global.h"
#include "../include/console.h"

#define TTY_FIRST    (tty_table)
#define TTY_END      (tty_table + NR_CONSOLES)

extern void enable_irq(int irq);

extern void disable_irq(int irq);

extern void milli_delay(int milli_sec);

extern void select_console(int nr_console);

extern void init_screen(TTY *p_tty);

extern int is_current_console(CONSOLE *p_con);

extern void flush(CONSOLE* p_con);

extern void keyboard_read(TTY *p_tty);

extern void move_cursor(CONSOLE *p_con, int destination);

PUBLIC void disp_insert();

PUBLIC void disp_insert_white();

PRIVATE void init_tty(TTY *p_tty);

PRIVATE void tty_do_read(TTY *p_tty);

//todo 修改
PRIVATE char tty_do_write(TTY *p_tty);

PRIVATE void put_key(TTY *p_tty, u32 key);

PRIVATE void change_mode();

PRIVATE void handle_printable(TTY *p_tty, u32 key);

PRIVATE void handle_unprintable(TTY *p_tty, u32 key);

PRIVATE void handle_enter(TTY *p_tty);

PRIVATE void handle_backspace(TTY *p_tty);

PRIVATE void handle_tab(TTY *p_tty);

PRIVATE void handle_simple(TTY *p_tty, u32 key);

PRIVATE void handle_search(TTY *p_tty, u32 key);

PRIVATE void escape_search();

PRIVATE void init_input();

PRIVATE void add_char(char c);

PUBLIC void clear_all();

PRIVATE char getData();

void move_cursor_up();

void move_cursor_down();

PRIVATE void move_cursor_left();

PRIVATE void move_cursor_right();

PRIVATE void move_cursor_start();

PRIVATE void move_cursor_end();

PRIVATE void move_cursor_line_start();

PRIVATE void move_cursor_line_end();

/*======================================================================*
  存储所有输入的内容，最后一个字符所在行和列
 *======================================================================*/
char input[50][80];
int row, column;

int search_mode;//查找模式
int search_done;//查找模式中按下回车，显示查找结果

int insert_mode;//插入模式

char search_content[512];//查找内容
int content_length;//查找内容长度

TTY *p_tty;

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty() {
    init_keyboard();

    for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
        init_tty(p_tty);
    }
    select_console(0);
    while (1) {
        for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
            tty_do_read(p_tty);
            tty_do_write(p_tty);
        }
    }
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY *p_tty) {
    p_tty->inbuf_count = 0;
    p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

    init_screen(p_tty);
    init_input();
}

/*初始化输入内容数组*/
PRIVATE void init_input() {
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 80; ++j) {
            input[i][j] = 0;
        }
    }
    row = 0;
    column = 0;

    search_done = 0;
    search_mode = 0;
    insert_mode = 0;
    for (int k = 0; k < sizeof(search_content); ++k) {
        search_content[k] = 0;
    }
    content_length = 0;
}

/*======================================================================*
				in_process
 *======================================================================*/
/*处理流程：
 * 1.判断是否是ESC
 *      若是，转入change_mode以改变模式
 *      返回
 * 2.判断是否处于search_done状态
 *      若是，屏蔽所有输入，直接返回（ESC步骤1中已经判断）
 * 3.判断是否处于search模式
 *      若是，转入handle_search
 *      否则，转入handle_simple*/
PUBLIC void in_process(TTY *p_tty, u32 key) {
    static char output[2] = {'\0', '\0'};

    int temp = key & MASK_RAW;
    if (temp == ESC) {
        change_mode();
        return;
    }

    if (search_done) {
        return;
    }

    if (search_mode) {
        handle_search(p_tty, key);
    } else {
        handle_simple(p_tty, key);
    }
}

/*改变模式：
 *  1.判断当前是否处于search模式
 *      若是，转入escape_search跳出search模式
 *  2.修改search模式，屏蔽/开启时钟中断*/
PRIVATE void change_mode() {
    if (search_mode) {
        escape_search();
        search_mode = 0;
        enable_irq(CLOCK_IRQ);
    } else {
        search_mode = 1;
        disable_irq(CLOCK_IRQ);
    }
}

/*跳出search模式
 *  1.删除search中输入的内容
 *  2.清空search_content
 *  3.重置search_done为0
 *  4.重新打印已有内容*/
PRIVATE void escape_search() {
    for (; content_length > 0;) {
        put_key(p_tty, '\b');
        search_content[--content_length] = 0;
    }
    search_done = 0;

    disp_pos = 0;
    int temp = p_tty->p_console->cursor;
    for (int j = 0; j <= temp / 80; ++j) {
        disp_color_str(input[j], 0x07);
    }
}


/*判断两个字符串是否相等*/
PRIVATE int str_equ(char *str_1, char *str_2, int length) {
    for (int i = 0; i < length; ++i) {
        if (*(str_1 + i) == 0) {
            return 0;
        }
        if (*(str_1 + i) != *(str_2 + i)) {
            return 0;
        }
    }

    return 1;
}


/*操作流程
 * 1.判断查询内容是否为空
 *      若是，结束操作
 * 2.循环判断每一行是否包含要查找的内容
 *   对每一个字符进行判断
 *      若不是，输出此字符
 *      若是，输出查找内容，同时移动指针*/
PRIVATE void search_process(TTY *p_tty) {
    if (!content_length) {
        return;
    }

    static char temp[] = {' ', '\0'};//用于输出单个字符

    disp_pos = 0;
    for (int i = 0; i <= row; i++) {
        for (int j = 0; j < 80; ++j) {
            if (input[i][j] == 0) {
                continue;
            }
            if (!str_equ(&input[i][j], search_content, content_length)) {
                temp[0] = input[i][j];
                disp_str(temp);
            } else {
                disp_color_str(search_content, 0x02);//绿色
                j = j + content_length - 1;
            }
        }
    }

    search_done = 1;
}


/*======================================================================*
				handle_search（查找处理）
 *======================================================================*/
/*处理流程：
 * 1.判断输入是否是回车
 *      若是，跳入search_process
 *      否则，存储输入内容，移动光标，打印输入内容*/
PRIVATE void handle_search(TTY *p_tty, u32 key) {
    disp_pos = p_tty->p_console->cursor * 2;
    if (!(key & FLAG_EXT)) {//可打印字符
        char ch = (char) key;
        search_content[content_length++] = ch;
        out_char(p_tty->p_console, ' ');//移动光标
        disp_color_str(search_content + content_length - 1, 0x02);//绿色
    } else {
        int raw_code = key & MASK_RAW;
        switch (raw_code) {
            case ENTER:
                search_process(p_tty);
                break;
            case TAB:
                for (int i = 0; i < 4; ++i) {
                    search_content[content_length++] = '\t';
                    disp_pos = p_tty->p_console->cursor * 2;
                    out_char(p_tty->p_console, ' ');//移动光标
                    disp_str(" ");
                }
                break;
            case BACKSPACE:
                if (content_length > 0) {
                    search_content[content_length--] = 0;
                    out_char(p_tty->p_console, '\b');
                    if (search_content[content_length] == '\t') {
                        for (int i = 0; i < 3; ++i) {
                            search_content[content_length--] = 0;
                            out_char(p_tty->p_console, '\b');
                        }
                    }
                }
                break;
        }
    }
}

/*处理插入*/
PRIVATE void handle_insert() {
    while (insert_mode) {
        disp_insert(0x70);
        milli_delay(5000);
        disp_insert(0x07);
        milli_delay(2000);
    }
}

/*打印黑底字符*/
PUBLIC void disp_insert(int color) {
    char ch = getData();
    if (ch == 0) {
        ch = ' ';
    }
    static char temp[] = {' ', '\0'};
    temp[0] = ch;
    disp_pos = p_tty->p_console->cursor * 2;
    disp_color_str(temp, color);
}


/*======================================================================*
				handle_simple（普通处理）
 *======================================================================*/
PRIVATE void handle_simple(TTY *p_tty, u32 key) {
    if (!(key & FLAG_EXT)) {
        handle_printable(p_tty, key);
    } else {
        handle_unprintable(p_tty, key);
    }
}


/*======================================================================*
				handle_printable（可打印字符处理）
 *======================================================================*/
PRIVATE void handle_printable(TTY *p_tty, u32 key) {
    char ch = (char) key;
    put_key(p_tty, key);
    add_char(ch);
}

/*======================================================================*
				handle_unprintable（不可打印字符处理）
 *======================================================================*/
PRIVATE void handle_unprintable(TTY *p_tty, u32 key) {
    int raw_code = key & MASK_RAW;
    switch (raw_code) {
        case ENTER:
            handle_enter(p_tty);
            break;
        case BACKSPACE:
            handle_backspace(p_tty);
            break;
        case TAB:
            handle_tab(p_tty);
            break;
        case INSERT:
            insert_mode ^= 1;
            handle_insert();
            break;
        case HOME:
            if ((key & FLAG_CTRL_L) || (key & FLAG_CTRL_R)) {
                move_cursor_start();
            } else {
                move_cursor_line_start();
            }
            break;
        case END:
            if ((key & FLAG_CTRL_L) || (key & FLAG_CTRL_R)) {
                move_cursor_end();
            } else {
                move_cursor_line_end();
            }
            break;
        case PAGEUP:
            for (int i = 0; i < 25; ++i) {
                scroll_screen(p_tty->p_console, SCR_UP);
            }
            break;
        case PAGEDOWN:
            for (int j = 0; j < 25; ++j) {
                scroll_screen(p_tty->p_console, SCR_DN);
            }
            break;
        case UP:
            if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                scroll_screen(p_tty->p_console, SCR_DN);
            } else {
                move_cursor_up();
            }
            break;
        case DOWN:
            if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                scroll_screen(p_tty->p_console, SCR_UP);
            } else {
                move_cursor_down();
            }
            break;
        case LEFT:
            move_cursor_left();
            break;
        case RIGHT:
            move_cursor_right();
            break;
        case F1:
        case F2:
        case F3:
        case F4:
        case F5:
        case F6:
        case F7:
        case F8:
        case F9:
        case F10:
        case F11:
        case F12:
            /* Alt + F1~F12 */
            if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
                select_console(raw_code - F1);
            }
            break;
        default:
            break;
    }
}

/*获得当前光标所在位置前的字符*/
PRIVATE char getData() {
    return input[p_tty->p_console->cursor / 80][p_tty->p_console->cursor % 80];
}

/*获取指定位置的字符*/
PRIVATE char getData_by_location(int location) {
    return input[location / 80][location % 80];
}

/*光标上移
 *  1.移动光标到上一行行尾
 *  2.判断目标位置是否存在字符
 *      若是，循环左移，直到目标位置
 *      否则，结束操作*/
void move_cursor_up() {
    int target = p_tty->p_console->cursor - 80;
    move_cursor_line_start();
    move_cursor_left();

    if (target > 0 && getData_by_location(target)) {
        int temp = p_tty->p_console->cursor - target;
        for (int i = 0; i < temp; ++i) {
            move_cursor_left();
        }
    }
}

/*光标下移
 *  1.移动光标至下一行行尾
 *  2.判断目标位置是否存在字符
 *      若是，循环左移至目标位置
 *      否则，结束操作*/
void move_cursor_down() {
    int target = p_tty->p_console->cursor + 80;
    move_cursor_line_end();
    move_cursor_right();
    move_cursor_line_end();

    if (getData_by_location(target)) {
        int temp =p_tty->p_console->cursor - target;
        for (int i = 0; i < temp; ++i) {
            move_cursor_left();
        }
    }
}

/*光标左移*/
PRIVATE void move_cursor_left() {
    do {
        move_cursor(p_tty->p_console, LEFT);
    } while (getData() == 0);
    if (getData() == '\t') {
        for (int i = 0; i < 3; ++i) {
            move_cursor(p_tty->p_console, LEFT);
        }
    }
}

/*光标右移
 *  1.判断光标所在位置是否为回车
 *      若是，移动到下一行行首
 *  2.判断是否存在字符
 *      若是，向右移动光标一次
 *      否则，结束操作
 *  3.判断是否是制表符
 *      若是，向右移动光标3次*/
PRIVATE void move_cursor_right() {
    int temp = p_tty->p_console->cursor;
    if (getData() == '\n') {
        for (int i = temp; i < (temp / 80 + 1) * 80; ++i) {
            move_cursor(p_tty->p_console, RIGHT);
        }
        return;
    }
    if (getData()) {
        move_cursor(p_tty->p_console, RIGHT);
    }
    if (getData() == '\t') {
        for (int i = 0; i < 3; ++i) {
            move_cursor(p_tty->p_console, RIGHT);
        }
    }
}


/*光标移动到行首
 * */
PRIVATE void move_cursor_line_start() {
    while (p_tty->p_console->cursor % 80 != 0) {
        move_cursor_left();
    }
}


/*光标移动到输入内容最开始的位置*/
PRIVATE void move_cursor_start() {
    while (p_tty->p_console->cursor) {
        move_cursor_line_start();
        move_cursor_left();
    }
}

/*光标移动到行尾
 *  处理流程：
 *      1.判断是否到达屏幕最右端
 *          若是，结束操作
 *      2.循环判断是否存在字符是不是回车
 *          若是，右移一位
 *          否则，结束操作*/
PRIVATE void move_cursor_line_end() {
    while (getData() && getData() != '\n') {
        if ((p_tty->p_console->cursor + 1) % 80 == 0) {//屏幕最右端
            break;
        }
        move_cursor_right();
    }
}

/*光标移动到输入内容最后位置
 *  处理流程：
 *      1.判断是否为回车
 *          若是，移动光标至下一行
 *      2.判断是否存在字符（是否为0）
 *          若存在，递归移动到最后位置
 *          否则，结束递归*/
PRIVATE void move_cursor_end() {
    while (getData()) {
        move_cursor_line_end();
        move_cursor_right();
    }
}

/*======================================================================*
				add_char（添加一个字符）
 *======================================================================*/
/*处理流程：
 *  1.判断此位置是否已存在字符
 *      若是，光标之后的字符逐个后移
 *  2.插入字符*/
PRIVATE void add_char(char c) {
    if (getData()) {//若已存在字符

    }
    input[row][column] = c;
    column++;
    if (column == 80) {
        column = 0;
        row++;
    }
}

/*清屏*/
PUBLIC void clear_all() {
    while (getData()) {
        move_cursor_right();
    }
    while (p_tty->p_console->cursor > 0) {
        out_char(p_tty->p_console, '\b');
    }
    init_input();
}

/*
 * 退格
 * 获取当前光标所在位置的前一个字符*/
PRIVATE char back() {
    char temp = input[row][column];
    input[row][column] = 0;

    if (column == 0) {
        if (row != 0) {
            column = 79;
            row--;
        }
    } else {
        column--;
    }
    return temp;
}

/*打印空格*/
PRIVATE void handle_space(int num) {
    for (int i = 0; i < num; ++i) {
        put_key(p_tty, ' ');
        add_char('\t');
    }
}

//RPIVATE int align() {
//    int lastLine = p_tty->p_console->cursor - 80;
//    for (int i = 0; i < 4; ++i) {
//        if (lastLine > 0 && )
//    }
//}

/*======================================================================*
				handle_tab（TAB处理）
 *======================================================================*/
PRIVATE void handle_tab(TTY *p_tty) {
    int lastLine = p_tty->p_console->cursor - 80;
    int align = 0;//记录是否需要纵向对齐

    if (lastLine >= 0) {
        for (int j = 3; j > 0; --j) {
            if (getData_by_location(lastLine - j) == '\t' && getData_by_location(lastLine + 4 - j) != '\t') {
                handle_space(4 - j);
                align = 1;
                break;
            }
        }
        if (!align) {
            for (int i = 0; i < 4; ++i) {
                if (getData_by_location(lastLine + i) == '\t') {
                    handle_space(4 + i);
                    align = 1;
                    return;
                }
            }
        }
    }

    if (!align) {
        handle_space(4);
    }
}

/*======================================================================*
				handle_enter（回车处理）
 *======================================================================*/
PRIVATE void handle_enter(TTY *p_tty) {
    put_key(p_tty, '\n');

    input[row][column] = '\n';
    row++;
    column = 0;
    int i = 0;
    while (input[row - 1][i] == '\t') {
        handle_tab(p_tty);
        i += 4;
    }
}

/*删除tab
 * handle_backspace中已退格一次，此处只退三次*/
PRIVATE void back_tab(TTY *p_tty) {
    for (int i = 0; i < 3; ++i) {
        put_key(p_tty, '\b');
        back();
    }
}


/*======================================================================*
				handle_backspace（退格处理）
 *======================================================================*/

/*判断光标前的字符
 * 1.若不为0,退一格;
 * 2.若为0,递归退格，直到不为0
 *退格包括：
 * 1.屏幕输出'\b'
 * 2.光标前字符置为0
 * */
PRIVATE void handle_backspace(TTY *p_tty) {
    if (p_tty->p_console->cursor == p_tty->p_console->original_addr) {
        return;
    }
    put_key(p_tty, '\b');
    back();
    if (input[row][column] == 0) {
        handle_backspace(p_tty);
    } else if (input[row][column] == '\t') {
        back_tab(p_tty);
    }
}


/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY *p_tty, u32 key) {
    if (p_tty->inbuf_count < TTY_IN_BYTES) {
        *(p_tty->p_inbuf_head) = key;
        p_tty->p_inbuf_head++;
        if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
            p_tty->p_inbuf_head = p_tty->in_buf;
        }
        p_tty->inbuf_count++;
    }
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY *p_tty) {
    if (is_current_console(p_tty->p_console)) {
        keyboard_read(p_tty);
    }
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE char tty_do_write(TTY *p_tty) {
    char ch = *(p_tty->p_inbuf_tail);
    if (p_tty->inbuf_count) {
        p_tty->p_inbuf_tail++;
        if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
            p_tty->p_inbuf_tail = p_tty->in_buf;
        }
        p_tty->inbuf_count--;

        out_char(p_tty->p_console, ch);
    }
    return ch;
}

