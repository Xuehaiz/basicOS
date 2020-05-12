CFLAGS = -Wall -W -g
CC = gcc
OBJECTS = part3.o # part2.o part3.o part4.o part5.o iobound.o cpubound.o
TARGETS = part3 # part2 part3 part4 part5 iobound cpubound
RM = rm -rf

all: $(TARGETS)

# part1: part1.o
# 	$(CC) $(CFLAGS) -o part1 part1.o

# part2: part2.o
# 	$(CC) $(CFLAGS) -o part2 part2.o

part3: part3.o
	$(CC) $(CFLAGS) -o part3 part3.o

# part4: part4.o
# 	$(CC) $(CFLAGS) -o part4 part4.o

# part5: part5.o
# 	$(CC) $(CFLAGS) -o part5 part5.o

# iobound: iobound.o
# 	$(CC) $(CFLAGS) -o iobound iobound.o

# cpubound.o: cpubound.c
# 	$(CC) $(CFLAGS) -o cpubound cpubound.o

# part1.o: part1.c

# part2.o: part2.c

part3.o: part3.c

# part4.o: part4.c

# part5.o: part5.c

# iobound.o: iobound.c

# cpubound.o: cpubound.c

clean:
	$(RM) $(TARGETS) $(OBJECTS)
