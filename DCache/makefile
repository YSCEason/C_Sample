#
# makefile for Audio_CModel CModel
#
#
CC = g++
#CCOPTIONS=-DWINDOWS -DFORCEINLINE -DMINGW -DSRTP_SUPPORT -D__EXPORT= -D_WIN32_WINNT=0x0501 -DNOMVS
INC = -I.
OLIST = main_b.o
# OLIST = main.o



: prepare1
dos: prepare1
# unix: prepare2

prepare1: clean DCache_test
	@echo "-Done-"

# prepare2: clean Audio_mainu
# 	@echo "-Done-"


DCache_test: $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@ -static-libgcc -static-libstdc++ -Wall

# main: $(OLIST)
# 	$(CC) $(INC) $(OLIST) -o ./bin/$@ -static-libgcc -static-libstdc++



main_b.o:
	$(CC) $(INC) ./main_b.cpp -c

# main.o:
# 	$(CC) $(INC) ./main.cpp -c



clean:
	rm -rf *.o