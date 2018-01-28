#
# makefile for Audio_CModel CModel
#
#
CC = g++
#CCOPTIONS=-DWINDOWS -DFORCEINLINE -DMINGW -DSRTP_SUPPORT -D__EXPORT= -D_WIN32_WINNT=0x0501 -DNOMVS
INC = -I.
# OLIST = main_b.o
OLIST = api.o
OLIST = ApiJtag.o
OLIST = dll.o
OLIST = dllmain.o
OLIST = fpga.o
OLIST = globals.o
OLIST = jtagport.o
OLIST = jtagproto.o
OLIST = lowarc.o
OLIST = lowio.o
OLIST = parport.o
OLIST = parproto.o
OLIST = port.o
OLIST = stdafx.o


: prepare1
dos: prepare1

prepare1: clean main_8k
	@echo "-Done-"




main_8k: $(OLIST)
	$(CC) $(INC) $(OLIST) -o ./bin/$@ -static-libgcc -static-libstdc++ -Wall





# main_b.o:
# 	$(CC) $(INC) ./main_b.cpp -c

api.o:
	$(CC) $(INC) ./api.cpp -c
ApiJtag.o:
	$(CC) $(INC) ./ApiJtag.cpp -c
dll.o:
	$(CC) $(INC) ./dll.cpp -c
dllmain.o:
	$(CC) $(INC) ./dllmain.cpp -c
fpga.o:
	$(CC) $(INC) ./fpga.cpp -c
globals.o:
	$(CC) $(INC) ./globals.cpp -c
jtagport.o:
	$(CC) $(INC) ./jtagport.cpp -c
jtagproto.o:
	$(CC) $(INC) ./jtagproto.cpp -c
lowarc.o:
	$(CC) $(INC) ./lowarc.cpp -c
lowio.o:
	$(CC) $(INC) ./lowio.cpp -c
parport.o:
	$(CC) $(INC) ./parport.cpp -c
parproto.o:
	$(CC) $(INC) ./parproto.cpp -c
port.o:
	$(CC) $(INC) ./port.cpp -c
stdafx.o:
	$(CC) $(INC) ./stdafx.cpp -c



clean:
	rm -rf *.o