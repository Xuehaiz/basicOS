cc = gcc

sources = $(wildcard *.c)
objects = $(addsuffix .o, $(basename $(sources)))

flags = -g -W -Wall -std=gnu99

target = pseudu-shell

all: $(target) 

$(target) : $(objects)
	$(cc) $(flags) -o $(target) $(objects)

%.o : %.c
	$(cc) -c $(flags) $< -o $@

clean:
	rm -rf $(target) $(objects) 
