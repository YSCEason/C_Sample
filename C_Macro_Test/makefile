#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = Macro_main.o



Macro_Test: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



Macro_main.o:
	$(CC) $(INC) ./Macro_main.c -c



clean:
	rm -rf *.o