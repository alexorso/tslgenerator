PROGRAM = tsl
BIN = /nfs/spectre/u6/callanse/bin

OBJS = main.o input.o output.o debug.o
HDRS = defines.h structs.h maindefs.h
LIBS = 

CC = gcc -Wall -g

$(PROGRAM): $(OBJS) $(HDRS)
	$(CC) -static -o $(PROGRAM) $(OBJS) $(LIBS)

clean:
	rm -f $(PROGRAM) $(OBJS) $(D_OBJS)
