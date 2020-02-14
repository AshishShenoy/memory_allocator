a.out: client.o allocator.o
	gcc client.o allocator.o

client.o: client.c allocator.h
	gcc -c client.c

allocator.o: allocator.c allocator.h
	gcc -c allocator.c

# Test with the client file.
test:
	./a.out