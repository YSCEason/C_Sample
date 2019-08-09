#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = LinkedList.o



LinkedList: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



LinkedList.o:
	$(CC) $(INC) ./LinkedList.c -c



clean:
	rm -rf *.o
