#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = Struct_pointer_main.o



Struct_Pointer_Test: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



Struct_pointer_main.o:
	$(CC) $(INC) ./Struct_pointer_main.c -c



clean:
	rm -rf *.o