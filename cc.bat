echo ON
prompt $g
cls
del %1.EXE
gcc -Ofast -Wpedantic -march=x86-64 -o %1.exe %1.c
rem
