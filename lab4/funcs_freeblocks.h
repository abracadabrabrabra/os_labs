#ifdef DLLFREEBLOCKS_EXPORTS
#define MYLIBRARY_API __declspec(dllexport)
#else
#define MYLIBRARY_API __declspec(dllimport)
#endif


#define MAX_SIZE 10000
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef struct Block {
    size_t free_space;
    size_t size;
    struct Block* next;
} FreeBlock;

typedef struct Allocator {
    Block* blocks;
    void* memory;
    size_t total_size;
} Allocator;

extern "C" MYLIBRARY_API void my_printf(const char* format, ...);

extern "C" MYLIBRARY_API Allocator* allocator_create(void* memory, const size_t size);

extern "C" MYLIBRARY_API void allocator_destroy(Allocator* allocator);

extern "C" MYLIBRARY_API void print(Block* block);

extern "C" MYLIBRARY_API void print_blocks(Allocator* allocator);

extern "C" MYLIBRARY_API void* allocator_alloc(Allocator* allocator, const size_t size);

extern "C" MYLIBRARY_API void allocator_free(Allocator* allocator, void* memory);

extern "C" MYLIBRARY_API void my_printf(const char* format, ...);
