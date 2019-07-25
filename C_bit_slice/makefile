#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = C_bit_slice.o



slice_Test: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



C_bit_slice.o:
	$(CC) $(INC) ./C_bit_slice.c -c



clean:
	rm -rf *.o
