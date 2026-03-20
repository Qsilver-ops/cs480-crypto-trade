#Makefile

# Specify compiler
CC = g++
# -std=c++11  C/C++ variant to use
# -Wall       show warning messages
# -g3         include information for symbolic debugger gdb
# -I./include tell compiler where to find header files
CCFLAGS = -std=c++11 -Wall -g3 -c -I./include

# Object files
OBJS = main.o shared.o producer.o consumer.o settler.o log.o

# Program name
PROGRAM = tradepipeline

# The program depends upon its object files
$(PROGRAM) : $(OBJS)
	$(CC) -pthread -o $(PROGRAM) $(OBJS)

main.o : src/main.cpp
	$(CC) $(CCFLAGS) src/main.cpp

shared.o : src/shared.cpp include/shared.h
	$(CC) $(CCFLAGS) src/shared.cpp

producer.o : src/producer.cpp include/producer.h
	$(CC) $(CCFLAGS) src/producer.cpp

consumer.o : src/consumer.cpp include/consumer.h
	$(CC) $(CCFLAGS) src/consumer.cpp

settler.o : src/settler.cpp include/settler.h
	$(CC) $(CCFLAGS) src/settler.cpp

log.o : src/log.c include/log.h
	$(CC) $(CCFLAGS) src/log.c

clean :
	rm -f $(OBJS) *~ $(PROGRAM)
