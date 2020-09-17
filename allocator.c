#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allocator.h"

// Global byte array, holds the allocated memory.
char *p;

// Global int, holds the size of the allocated memory.
int size;

struct Book {
    // Capacity of the memory managed by the book.
    unsigned int capacity;

    // Determines if the memory managed is allocated (true) or free (false).
    bool isAllocated;

    // Pointer to the next book, implemented in a linked list manner.
    // Value is NULL if no book follows.
    char *nextBookLocation;
};

typedef struct Book book_t;

void allocate(int n) {
    // Manages all the free space in the initial allocated
    // memory (After subtracting the space required for the book).
    book_t init_book;

    // If memory requested is less than the size of the book structure, then do
    // not allocate memory.
    if (n < sizeof(book_t)) {
        printf(
            "Memory allocated insufficient for the book keeping structure. "
            "Please allocate more.\n");
        exit(EXIT_FAILURE);
    }

    // Allocates required memory to the byte array.
    p = (char *)malloc(n * sizeof(char));
    size = n;

    init_book.nextBookLocation = NULL;
    init_book.capacity = n - sizeof(book_t);
    init_book.isAllocated = false;

    // Copies the book over to the byte array.
    memcpy(p, (char *)&init_book, sizeof(book_t));
}

// The parameter was renamed from size to requested_size to
// avoid clash with the global size variable.
void *mymalloc(int requested_size) {
    // Loop variable, used to iterate over all the books present in the
    // allocated memory. Also holds the book for the allocatable memory block.
    book_t *book = NULL;

    // Book which manages the leftover memory block after the allocated block.
    book_t *following_block = NULL;

    // Pointer to the full allocated memory block, used to obtain the loop book variable.
    char *book_pointer = NULL;

    // Holds the size of the smallest possible allocatable block of memory. (Best Fit)
    int minimum_capacity = size;

    // Holds the location of the smallest possible allocatable block.
    char *malloc_block = NULL;

    // Boolean, determines if it is possible to allocate a block of memory
    // of the requested size.
    bool isAllocatable = false;

    // Exit if the requested memory size is negative.
    if (requested_size >= 0) {
        // Iterate over all books in the complete allocated memory, and find the
        // best fit free memory block.
        for (book_pointer = p; book_pointer != NULL; book_pointer = book->nextBookLocation) {
            book = (book_t *)(book_pointer);
            if (!book->isAllocated) {
                if (book->capacity >= requested_size && book->capacity < minimum_capacity) {
                    isAllocatable = true;
                    malloc_block = book_pointer;
                    minimum_capacity = book->capacity;
                }
            }
        }

        if (isAllocatable) {
            // Read the book available at the allocatable block.
            book = (book_t *)(malloc_block);

            // Create a new book after the allocated block, pointing to the leftover free
            // memory. Only takes place if enough memory is present, else the leftover memory is
            // part of the newly allocated block to the left.
            if ((malloc_block + requested_size - p) > sizeof(book_t)) {
                following_block = (book_t *)(malloc_block + requested_size + sizeof(book_t));
                following_block->isAllocated = false;
                following_block->capacity = book->capacity - requested_size - sizeof(book_t);
                following_block->nextBookLocation = book->nextBookLocation;

                book->capacity = requested_size;
            }

            // Update the allocated block's book.
            book->isAllocated = true;
            book->nextBookLocation = malloc_block + sizeof(book_t) + requested_size;

            return (malloc_block + sizeof(book_t));
        }
    } else
        exit(EXIT_FAILURE);
}

void myfree(void *b) {
    // Loop variable, used to iterate over all the books present in the
    // allocated memory.
    book_t *book = NULL;

    // Book which manages the memory before the freed block of memory.
    // Merges with the freed block if its memory is not allocated.
    book_t *prev_book = NULL;

    // Book which manages the memory after the freed block of memory.
    // Merges with the freed block if its memory is not allocated.
    book_t *next_book = NULL;

    // Pointer to the full allocated memory block, used to obtain the loop
    // book variable.
    char *book_pointer = NULL;

    // Boolean, determines if the memor block is valid for freeing.
    bool isValidBlock = false;

    // If the address provided is NULL, crash the program.
    if (b != NULL) {
        // Read the book corresponding to the pointer given, and also set
        // the value of prev_book to the book previous to the block being freed.
        for (book_pointer = p; book_pointer != NULL; book_pointer = book->nextBookLocation) {
            prev_book = book;
            book = (book_t *)(book_pointer);

            if (book_pointer + sizeof(book_t) == b) {
                isValidBlock = true;
                break;
            }
        }

        // Return if the pointer doesn't point to a valid block.
        if (!isValidBlock) return;

        // If the address provided points to a free block of memory, then do nothing.
        if (!book->isAllocated) return;

        // Free up the memory block.
        book->isAllocated = false;

        // Merges with the block preceding the freed block if not allocated.
        if (prev_book && !prev_book->isAllocated) {
            prev_book->nextBookLocation = book->nextBookLocation;
            prev_book->capacity += (sizeof(book_t) + book->capacity);
            book = prev_book;
        }

        // Checks for a valid memory block (with a book), following the freed block.
        if (book->nextBookLocation) {
            next_book = (book_t *)(book->nextBookLocation);

            // Merges with the freed block if not allocated.
            if (!next_book->isAllocated) {
                book->nextBookLocation = next_book->nextBookLocation;
                book->capacity += (next_book->capacity + sizeof(book_t));
                book = next_book;
            }
        }

    } else
        exit(EXIT_FAILURE);
}

void print_book() {
    // Returns size of the structure used for book keeping.
    printf("%ld\n", sizeof(book_t));
}

void display_mem_map() {
    // Loop variable, used to iterate over all the books present in the allocated memory.
    book_t book;
    book.nextBookLocation = p;

    // Holds the book before the loop variable, or the beginning of the allocated memory.
    book_t prev_book;

    // While the book is present, print its relative start and bytes occupied.
    // Also print the relative start and bytes occupied of the memory managed
    // by the book, along with its status (allocated or free).
    do {
        prev_book = book;

        // Moves to the next book present in the allocated memory.
        memcpy((book_t *)&book, book.nextBookLocation, sizeof(book_t));

        printf("%ld\t%ld\tbook\n", prev_book.nextBookLocation - p, sizeof(book_t));

        if (book.isAllocated)
            printf("%ld\t%d\tallocated\n", (prev_book.nextBookLocation + sizeof(book_t)) - p,
                   book.capacity);
        else
            printf("%ld\t%d\tfree\n", (prev_book.nextBookLocation + sizeof(book_t)) - p,
                   book.capacity);

    } while (book.nextBookLocation != NULL);
}
