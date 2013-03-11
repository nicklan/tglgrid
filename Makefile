CC=gcc
CFLAGS=-I/usr/include/pdextended -fPIC # -g

default: tglgrid.pd_linux

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%.pd_linux: %.o
	ld --export-dynamic -shared -o $@ $< -lc -lm

clean:
	rm *.o *.pd_linux
