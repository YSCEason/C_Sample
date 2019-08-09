#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = BubbleSort.o



BubbleSort: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



BubbleSort.o:
	$(CC) $(INC) ./BubbleSort.c -c



clean:
	rm -rf *.o
