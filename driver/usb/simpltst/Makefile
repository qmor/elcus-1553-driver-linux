# Lucifer V1.0
# By Peter Todd (c) 1999
#
# Note that the dos edit command doesn't work. When your press tab it 
# inserts spaces instead of a tab. :(

CC = gcc
CFLAGS = -g -Wall 

all: simpltst.o
	$(CC) $(CFLAGS) -lm -o simpltst simpltst.o

simpltst.o: simpltst.c ltmk.c
	$(CC) $(CFLAGS) -c simpltst.c -D_TMK1553B_LINUX

clean:
	rm *.o
	rm simpltst

realclean: clean
	rm *~


