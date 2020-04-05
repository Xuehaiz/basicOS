cc = gcc

sources = $(wildcard *.c)
objects = $(addsuffix .o, $(basename $(sources)))

flags = -g -W -Wall -std=c11

target = lab3.exe

all: $(target)

$(target) : $(objects)
	$(cc) $(flags) -o $(target) $(objects)

%.o : %.c
	$(cc) -c $(flags) $< -o $@

clean:
	rm -rf $(target) $(objects)