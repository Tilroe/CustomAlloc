#include <stdio.h>

#include "alloc.h"

int main(void) {
    // 1. Simple alloc/free case
    // When freeing, should consume the next block (FREE block that stretches to the end of memory) (alloc.c line 99)
    int items = 8;
    int* test = (int*)alloc_custom(sizeof(int) * items);
    free_custom(test);

    // 2. Three key paths taken here
    //      2.1: allocating block2 jumps over block1 allocation (because it is allocated)
    //      2.2: freeing block1 does not give it any extra block space, because the next block (block2) is allocated
    //      2.3: not enough free space in first free block, jumps past block2
    int items_block1 = 4, items_block2 = 4, items_block3 = 6;
    int* block1 = (int*)alloc_custom(sizeof(int) * items_block1);
    int* block2 = (int*)alloc_custom(sizeof(int) * items_block2); // 2.1
    free_custom(block1); // 2.2
    int* block3 = (int*)alloc_custom(sizeof(int) * items_block3); // 2.3
    free_custom(block2);
    free_custom(block3);

    // 3. Not enough space case
    // *On my machine, size_t is 8 bytes, and ints are 4 bytes
    // 1024 byte memory - 9 bytes initial metadata = 1015 bytes available initially
    // 1015 bytes / 4 bytes per int = 253.75 int
    // So trying to allocate for 254 ints should fail (alloc.c line 78)
    int* should_fail = (int*)alloc_custom(sizeof(int) * 254);

    // 4. Working with metadata (or lack thereof) at the end of memory
    //      4.1: Allocating 253 ints would leave 3 free bytes left at the end of memory
    //          When we try to indicate that those 3 bytes are free by writing metadata for a FREE block, it fails
    //          because 3 bytes is not enough for a 9 byte metadata (alloc.c line 83 call exits early (alloc.c line 58))
    //      4.2: When freeing the above block, we try to read the next metadata to see if we can combine with a free
    //          block. The metadata read fails because it would go beyond the limits of memory (alloc.c line 97 exits
    //          early (alloc.c line 46)). We then know this block is at the end of memory, so when freeing we give
    //          it the extra 3 bytes of memory at the end
    int* big_block = (int*)alloc_custom(sizeof(int) * 253); // 4.1
    free_custom(big_block); // 4.2

    return 0;
}