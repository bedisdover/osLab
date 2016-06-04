cmake_minimum_required(VERSION 3.5)
project(lab4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    boot/include/fat12hdr.inc
    boot/include/load.inc
    boot/include/pm.inc
    boot/include/pm.incnc
    boot/boot.asm
    boot/boot.bin
    boot/loader.asm
    boot/loader.bin
    include/const.h
    include/global.h
    include/proc.h
    include/protect.h
    include/proto.h
    include/sconst.inc
    include/string.h
    include/type.h
    kernel/clock.c
    kernel/global.c
    kernel/i8259.c
    kernel/kernel.asm
    kernel/main.c
    kernel/proc.c
    kernel/protect.c
    kernel/start.c
    kernel/syscall.asm
    lib/klib.c
    lib/kliba.asm
    lib/string.asm
    a.img
    bochsrc
    kernel.bin
    lab4.odt
    Makefile
    readme.md
    实验4_TTY任务框架.pptx
    操作系统实验四.pdf)

add_executable(lab4 ${SOURCE_FILES})