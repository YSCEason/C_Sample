gcc -c -o sum.o sum.c

ar -rcs libsum.a sum.o

gcc main.c -L. -lsum -o main_static