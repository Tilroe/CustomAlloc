customalloc: main.c alloc.o
	$(CC) -o customalloc main.c alloc.o -g

alloc.o: alloc.h alloc.c
	$(CC) -c alloc.c -g

clean:
	rm *.o customalloc