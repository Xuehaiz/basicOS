all: server

server: quacker.o topicStore.o
	gcc -g -pthread -o server  quacker.o topicStore.o

quacker.o: quacker.h quacker.c topicStore.h
	gcc -g -pthread -c -o quacker.o quacker.c

topicStore.o: topicStore.h topicStore.c
	gcc -g -pthread -c -o topicStore.o topicStore.c

clean:
	rm -f quacker.o topicStore.o server