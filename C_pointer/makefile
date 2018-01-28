#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = Pointer_main.o



Pointer_Test: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



Pointer_main.o:
	$(CC) $(INC) ./Pointer_main.c -c



clean:
	rm -rf *.o