#
# makefile for pointer test
#
#

CC = gcc
INC = -I.
OLIST = lcd_tg_pattern.o



lcd_tg_pattern_Test: clean $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@



lcd_tg_pattern.o:
	$(CC) $(INC) ./lcd_tg_pattern.c -c



clean:
	rm -rf *.o
