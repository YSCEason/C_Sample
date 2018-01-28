#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = FunPointer.o



FunPointer: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



FunPointer.o:
	$(CC) $(INC) ./FunPointer.c -c



clean:
	rm -rf *.o