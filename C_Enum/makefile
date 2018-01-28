#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = Enum_main.o



Enum_Test: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



Enum_main.o:
	$(CC) $(INC) ./Enum_main.c -c



clean:
	rm -rf *.o