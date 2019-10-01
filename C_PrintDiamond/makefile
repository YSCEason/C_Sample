#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = PrintDiamond.o



PrintDiamond_Test: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



PrintDiamond.o:
	$(CC) $(INC) ./PrintDiamond.c -c



clean:
	rm -rf *.o
