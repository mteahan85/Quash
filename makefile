Quash: main.o
	gcc main.o -o Quash

main.o: main.c
	gcc -c -g main.c

clean:
	rm -f *.o Quash
