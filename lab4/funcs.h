#ifdef DLLPOW2_EXPORTS
#define MYLIBRARY_API __declspec(dllexport)
#else
#define MYLIBRARY_API __declspec(dllimport)
#endif

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>

#define N_BLOCKS 10
#define NUM_LISTS 11          // Количество списков (от 0 до 10)


typedef struct Block {
    struct Block* next;
    struct Block* prev;
    size_t size;
} Block;

typedef struct Allocator {
    Block* freeLists[NUM_LISTS];
    void* memory;
    size_t total_size;
    size_t start_extent;
} Allocator;

extern "C" MYLIBRARY_API void my_printf(const char* format, ...);

extern "C" MYLIBRARY_API int power(int base, int exp);

extern "C" MYLIBRARY_API void print_blocks(Allocator* a);

extern "C" MYLIBRARY_API Allocator* allocator_create(void* mem, size_t size);

extern "C" MYLIBRARY_API void split_block(Allocator* allocator, Block* block);

extern "C" MYLIBRARY_API void* allocator_alloc(Allocator* allocator, size_t size);

extern "C" MYLIBRARY_API void allocator_free(Allocator* allocator, void* ptr);

extern "C" MYLIBRARY_API void allocator_destroy(Allocator* allocator);
