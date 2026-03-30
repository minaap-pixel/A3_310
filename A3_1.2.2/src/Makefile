CC=gcc
#CFLAGS=-g -O0 -Wall #-DNDEBUG
CFLAGS=-DNDEBUG -Wall

# default values if only make mysh is called. framesize is a multiple of 3
framesize ?= 18
varmemsize ?= 10

# replace the macros framesize and varmemsize with the values in the command line
MACROS = -DFRAME_STORE_SIZE=$(framesize) -DVAR_STORE_SIZE=$(varmemsize)

OBJECT_FILES = shell.o interpreter.o shellmemory.o pcb.o queue.o schedule_policy.o

# this is practically the same as the original Makefile part except all
# the object files were put into a single variable:
# $(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o pcb.o queue.o schedule_policy.o 
	
mysh: $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(MACROS) -o mysh $(OBJECT_FILES)

# calling the original .c files one by one to compile them into .o files
%.o: %.c
	$(CC) $(CFLAGS) $(MACROS) -c $< -o $@

clean: 
	rm -f mysh *.o
