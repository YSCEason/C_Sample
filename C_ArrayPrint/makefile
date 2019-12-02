#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = ArrayPrint.o



ArrayPrint_Test: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



ArrayPrint.o:
	$(CC) $(INC) ./ArrayPrint.c -c



clean:
	rm -rf *.o
