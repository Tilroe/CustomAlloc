#ifndef CUSTOMALLOC_ALLOC_H
#define CUSTOMALLOC_ALLOC_H

#include <stddef.h>

char* alloc_custom(size_t size);
void free_custom(void* ptr);

#endif //CUSTOMALLOC_ALLOC_H