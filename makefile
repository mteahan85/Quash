Quash: main.o
	gcc main.o -o Quash -lreadline

main.o: main.c
	gcc -c -g -w -std=c99 main.c -lreadline


clean:
	rm -f *.o Quash
