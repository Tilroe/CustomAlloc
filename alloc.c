/** alloc.c
 * My custom memory allocator
 * Takes the approach of prepending any block of memory with some metadata, which indicates how large that block
 * is, and whether it is free or allocated
 */

#include "alloc.h"

#include <string.h>

#define MEMORY_SIZE 1024
#define METADATA_SIZE (sizeof(size_t) + 1) // block size + 1 byte for FREE/ALLOCATED

struct block_metadata
{
    size_t block_size;
    enum {FREE, ALLOCATED} block_type;
};


// ----- Lazy initialization of memory -----
static void init_custom();
static size_t initial_size = MEMORY_SIZE - METADATA_SIZE;
static char initialized = 0;


// ----- Memory block -----
static char memory[MEMORY_SIZE] = {};

// ----- Helper functions -----

static char* read_metadata(const char* ptr, struct block_metadata* metadata);
static char* write_metadata(char* ptr, struct block_metadata metadata);

// Write first bit of metadata, indicating whole of memory is FREE
static void init_custom() {
    const struct block_metadata metadata = {initial_size, FREE};
    write_metadata(memory, metadata);
    initialized = 1;
}

// Read metadata from memory
// Returns ptr to start of metadata if read
// Returns NULL if this function would read outside of memory range
char* read_metadata(const char* ptr, struct block_metadata* metadata) {
    if ((ptr - memory) > (MEMORY_SIZE - METADATA_SIZE)) return NULL;
    size_t block_size;
    memcpy(&block_size, ptr, sizeof(size_t));
    metadata->block_size = block_size;
    metadata->block_type = *(ptr + sizeof(size_t));
    return ptr;
}

// Write metadata to memory
// Returns ptr to start of metadata if written
// Returns NULL if this function would write outside of memory range
char* write_metadata(char* ptr, struct block_metadata metadata) {
    if ((ptr - memory) > (MEMORY_SIZE - METADATA_SIZE)) return NULL;
    memcpy(ptr, &metadata.block_size, sizeof(size_t));
    ptr[sizeof(size_t)] = (char)metadata.block_type;
    return ptr;
}

// ----- Function definitions -----

char* alloc_custom(const size_t size) {
    if (!initialized) init_custom(); // Lazy initialization
    struct block_metadata metadata;

    // Searching consecutive metadata to find one that's free and big enough
    char *md_p = memory;
    while (
            (md_p = read_metadata(md_p, &metadata)) &&
            (metadata.block_type == ALLOCATED || metadata.block_type == FREE && metadata.block_size < size)
          ) {
        md_p += metadata.block_size + METADATA_SIZE;
    }
    if (!md_p) return NULL; // No free block big enough

    // Update metadata markings
    size_t old_block_size = metadata.block_size;
    write_metadata(md_p, (struct block_metadata) {size, ALLOCATED});
    write_metadata(md_p + METADATA_SIZE + size, (struct block_metadata) {old_block_size - METADATA_SIZE - size, FREE});
    return md_p + METADATA_SIZE;
}

void free_custom(void* ptr) {
    // Undefined behaviour if not freeing pointer given by allocator function
    if (!initialized) init_custom(); // Lazy initialization
    struct block_metadata metadata, next_metadata;

    // Reading metadata behind data pointer
    char* md_p = read_metadata(ptr - METADATA_SIZE, &metadata);
    if (metadata.block_type != ALLOCATED) return;

    // Checking if we can combine with next memory block (if it's free)
    const char* nmd_p = read_metadata(ptr + metadata.block_size, &next_metadata);
    size_t extra_block_size;
    if (nmd_p) extra_block_size = (nmd_p && next_metadata.block_type == FREE) ? METADATA_SIZE + next_metadata.block_size : 0; // Consuming next free memory block
    else extra_block_size = (memory + MEMORY_SIZE) - ((char*)ptr + metadata.block_size); // Extra little bit at the end that couldn't fit another allocation

    // Updating metadata
    const struct block_metadata updated_metadata = {metadata.block_size + extra_block_size, FREE};
    write_metadata(md_p, updated_metadata);
}
