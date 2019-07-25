#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = simple.o ../jsmn.h



simple: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./$@



simple.o:
	$(CC) $(INC) ./simple.c -c



clean:
	rm -rf *.o
