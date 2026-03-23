#Makefile

# Specify compiler
CC = g++
# -std=c++11  C/C++ variant to use
# -Wall       show warning messages
# -g3         include information for symbolic debugger gdb
# -I./include tell compiler where to find header files
CCFLAGS = -std=c++11 -Wall -g3 -pthread -c

# Object files
OBJS = main.o shared.o producer.o consumer.o settler.o log.o

# Program name
PROGRAM = tradepipeline

# The program depends upon its object files
$(PROGRAM) : $(OBJS)
	$(CC) -pthread -o $(PROGRAM) $(OBJS)

main.o : main.cpp
	$(CC) $(CCFLAGS) main.cpp

shared.o : shared.cpp shared.h
	$(CC) $(CCFLAGS) shared.cpp

producer.o : producer.cpp producer.h
	$(CC) $(CCFLAGS) producer.cpp

consumer.o : consumer.cpp consumer.h
	$(CC) $(CCFLAGS) consumer.cpp

settler.o : settler.cpp settler.h
	$(CC) $(CCFLAGS) settler.cpp

log.o : log.cpp log.h
	$(CC) $(CCFLAGS) log.cpp

clean :
	rm -f $(OBJS) *~ $(PROGRAM)
