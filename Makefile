#Makefile

# C++ Specify compiler
CC = g++
# Compiler flags
# -std=c++11  C/C++ variant to use
# -Wall       show warning messages
# -g3         include information for symbolic debugger gdb
# -I./include tell compiler where to find header files
CCFLAGS = -std=c++11 -Wall -g3 -c -I./include

# Object files .o files
OBJS = main.o shared.o producer.o consumer.o settler.o log.o

# Program name
PROGRAM = tradepipeline

# The program depends upon its object files
$(PROGRAM) : $(OBJS)
	$(CC) -pthread -o $(PROGRAM) $(OBJS)

# compile main.cpp to main.0
main.o : src/main.cpp
	$(CC) $(CCFLAGS) src/main.cpp

# compile shared.cpp to shared.o
shared.o : src/shared.cpp include/shared.h
	$(CC) $(CCFLAGS) src/shared.cpp

# compile producer.cpp to porducer.o
producer.o : src/producer.cpp include/producer.h
	$(CC) $(CCFLAGS) src/producer.cpp

# compile consumer.cpp to consumer.o
consumer.o : src/consumer.cpp include/consumer.h
	$(CC) $(CCFLAGS) src/consumer.cpp

# compile settler.cpp to settler.o
settler.o : src/settler.cpp include/settler.h
	$(CC) $(CCFLAGS) src/settler.cpp

# compile log.cpp to log.o
log.o : src/log.c include/log.h
	$(CC) $(CCFLAGS) src/log.c

# removes compiled files and executable
clean :
	rm -f $(OBJS) *~ $(PROGRAM)
