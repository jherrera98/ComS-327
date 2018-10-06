all: main

main: main.o
	gcc main.o -o main 

main.o: main.c
	gcc -c main.c -Wall -Werror -ggdb -o main.o

clean: 
	rm -f main.o cell.o main *~
