cmake_minimum_required(VERSION 3.5)
project(m)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    boot/include/fat12hdr.inc
    boot/include/load.inc
    boot/include/pm.inc
    boot/boot.asm
    boot/boot.bin
    boot/loader.asm
    boot/loader.bin
    include/console.h
    include/const.h
    include/global.h
    include/keyboard.h
    include/keymap.h
    include/proc.h
    include/protect.h
    include/proto.h
    include/sconst.inc
    include/string.h
    include/tty.h
    include/type.h
    kernel/clock.c
    kernel/console.c
    kernel/global.c
    kernel/i8259.c
    kernel/kernel.asm
    kernel/keyboard.c
    kernel/main.c
    kernel/proc.c
    kernel/protect.c
    kernel/start.c
    kernel/syscall.asm
    kernel/tty.c
    lib/klib.c
    lib/kliba.asm
    lib/string.asm
    bochsrc
    Makefile)

add_executable(m ${SOURCE_FILES})