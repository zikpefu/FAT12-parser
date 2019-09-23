CC=gcc
CFLAGS=-Wall -g
BINS= njc

all: $(BINS)
njc: 
	$(CC) $(CFLAGS) notjustcats.c -o notjustcats

clean:
			rm $(BINS)
%: %.c
					$(CC) $(CFLAGS) -o $@ $<
project:
		rm -f project4.tgz
		tar cvzf project4.tgz makefile notjustcats.c  README list.h
simple_1:
		rm -rf test
		mkdir test
		./notjustcats simple.img test
	
simple_2:
		rm -rf test2
		mkdir test2
		./notjustcats simple2.img test2

my_test:
		rm -rf tmp
		mkdir tmp
		./notjustcats tmp.img tmp