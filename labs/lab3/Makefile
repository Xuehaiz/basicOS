cc = gcc

sources = $(wildcard *.c)
objects = $(addsuffix .o, $(basename $(sources)))

<<<<<<< HEAD
flags = -g -W -Wall -std=gnu99 
=======
flags = -g -W -Wall 
>>>>>>> f8aca58e85efcc0671a8a4885ed923616abad672

target = lab3.exe

all: $(target) 

$(target) : $(objects)
	$(cc) $(flags) -o $(target) $(objects)

%.o : %.c
	$(cc) -c $(flags) $< -o $@

clean:
	rm -rf $(target) $(objects) 
