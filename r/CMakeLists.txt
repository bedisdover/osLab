cmake_minimum_required(VERSION 3.5)
project(r)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    boot/include/fat12hdr.inc
    boot/include/load.inc
    boot/include/pm.inc
    boot/boot.asm
    boot/loader.asm
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
    Makefile)

add_executable(r ${SOURCE_FILES})