#!/usr/bin/env bash
nasm -f elf -o my_print.o my_print.asm
gcc -c -o similarity.o similarity.c
gcc -o main main.c my_print.o similarity.o
./main
