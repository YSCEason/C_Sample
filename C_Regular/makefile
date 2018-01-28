#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = Regular_main.o



Enum_Test: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



Enum_main.o:
	$(CC) $(INC) ./Regular_main.c -c



clean:
	rm -rf *.o