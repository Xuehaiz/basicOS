CFLAGS = -Wall -W -g
CC = gcc
OBJECTS = part1.o part2.o part3.o part4.o 
TARGETS = part1 part2 part3 part4 
RM = rm -rf

all: $(TARGETS)

part1: part1.o
	$(CC) $(CFLAGS) -o part1 part1.o

part2: part2.o
	$(CC) $(CFLAGS) -o part2 part2.o

part3: part3.o
	$(CC) $(CFLAGS) -o part3 part3.o

part4: part4.o
	$(CC) $(CFLAGS) -o part4 part4.o

part1.o: part1.c

part2.o: part2.c

part3.o: part3.c

part4.o: part4.c

clean:
	$(RM) $(TARGETS) $(OBJECTS)
