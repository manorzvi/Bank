# Makefile for the bank program
CC = g++
CFLAGS = -std=c++11 -g -Wall -pthread -lpthread 
CCLINK = $(CC)
OBJS = bank.o Account.o
RM = rm -f
# Creating the  executable
bank: $(OBJS)
	$(CCLINK) $(CFLAGS) -o bank $(OBJS)
# Creating the object files
Account.o: Account.cpp Account.h
bank.o: bank.cpp
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*