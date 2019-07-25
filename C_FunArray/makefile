#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = FunArray.o



FunArray: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



FunArray.o:
	$(CC) $(INC) ./FunArray.c -c



clean:
	rm -rf *.o
