#include "pch.h"
#include "funcs.h"

extern "C" MYLIBRARY_API void my_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[1024];
    char* buf_ptr = buffer;
    const char* fmt_ptr = format;
    int buffer_size = sizeof(buffer);

    while (*fmt_ptr) {
        if (*fmt_ptr == '%') {
            fmt_ptr++;
            switch (*fmt_ptr) {
            case 'd': {
                int value = va_arg(args, int);
                char num_buffer[20];
                char* num_ptr = num_buffer;
                if (value < 0) {
                    *buf_ptr++ = '-';
                    value = -value;
                }
                do {
                    *num_ptr++ = (char)((value % 10) + '0');
                    value /= 10;
                } while (value > 0);
                while (num_ptr > num_buffer) {
                    *buf_ptr++ = *--num_ptr;
                }
                break;
            }
            case 'l': {
                unsigned long long value = va_arg(args, unsigned long long);
                char num_buffer[40];
                char* num_ptr = num_buffer;
                do {
                    *num_ptr++ = (char)((value % 10) + '0');
                    value /= 10;
                } while (value > 0);
                while (num_ptr > num_buffer) {
                    *buf_ptr++ = *--num_ptr;
                }
                break;
            }
            case 'k': {
                double value = va_arg(args, double);
                char num_buffer[20];
                char* num_ptr = num_buffer;
                if (value < 0) {
                    *buf_ptr++ = '-';
                    value = -value;
                }

                int afterDot = (value - (int)value) * 100000000;
                int beforeDot = (int)value;

                do {
                    *num_ptr++ = (char)((afterDot % 10) + '0');
                    afterDot /= 10;
                } while (afterDot > 0);

                *num_ptr++ = '.';

                do {
                    *num_ptr++ = (char)((beforeDot % 10) + '0');
                    beforeDot /= 10;
                } while (beforeDot > 0);

                while (num_ptr > num_buffer) {
                    *buf_ptr++ = *--num_ptr;
                }
                break;
            }
            case 's': {
                char* str = va_arg(args, char*);
                while (*str) {
                    *buf_ptr++ = *str++;
                }
                break;
            }
            case 'c': {
                char ch = (char)va_arg(args, int);
                *buf_ptr++ = ch;
                break;
            }
            case '%': {
                *buf_ptr++ = '%';
                break;
            }
            default:
                *buf_ptr++ = *fmt_ptr;
                break;
            }
        }
        else {
            *buf_ptr++ = *fmt_ptr;
        }
        fmt_ptr++;
    }
    *buf_ptr = '\0';

    va_end(args);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD bytesWritten;
    WriteConsoleA(hConsole, buffer, (DWORD)(buf_ptr - buffer), &bytesWritten, NULL);
}

extern "C" MYLIBRARY_API int power(int base, int exp) {
    long long result = 1;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result *= base;
        }
        base *= base;
        exp /= 2;
    }
    return result;
}

extern "C" MYLIBRARY_API void print_blocks(Allocator* a) {
    my_printf("Allocator size: %d\n", sizeof(Allocator));
    my_printf("Allocator memory: %d\n", a->memory);
    for (int i = 0; i < NUM_LISTS; i++) {
        my_printf("list %d: ", i);
        if (a->freeLists[i] == NULL) {
            my_printf("NULL\n");
        }
        else {
            Block* block = a->freeLists[i];
            while (block != NULL) {
                my_printf("block size %d offset %d | ", block->size, (char*)block - (char*)a->memory);
                block = block->next;
            }
        }
        my_printf("\n");
    }
    my_printf("\n");
}

extern "C" MYLIBRARY_API Allocator* allocator_create(void* mem, size_t size) {

    Allocator* allocator = (Allocator*)mem;
    allocator->total_size = size - sizeof(Allocator);
    allocator->memory = (char*)mem + sizeof(Allocator);

    for (int i = 0; i < NUM_LISTS; ++i)
        allocator->freeLists[i] = NULL;

    size_t extent = 0;

    while (power(2, extent) <= sizeof(Block))
        extent++;

    allocator->start_extent = extent;

    size_t offset = 0, pos = 0;
    size_t p = power(2, extent);
    size_t blocks_count = 0;

    while (offset + p <= allocator->total_size) {
        Block* block = (Block*)((char*)allocator->memory + offset);
        block->prev = NULL;
        if (allocator->freeLists[pos] == NULL) {
            block->next = NULL;
        }
        else {
            allocator->freeLists[pos]->prev = block;
            block->next = allocator->freeLists[pos];
        }
        allocator->freeLists[pos] = block;
        block->size = p - sizeof(Block);
        my_printf("Block size = %d, offset = %d\n", block->size, offset);
        offset += p;
        blocks_count++;
        if (blocks_count == N_BLOCKS) {
            pos++;
            extent++;
            p = power(2, extent);
            blocks_count = 0;
        }
    }
    my_printf("\n");

    return allocator;
}

extern "C" MYLIBRARY_API void split_block(Allocator* allocator, Block* block) {
    int index = 0;
    while ((1 << index) < block->size + sizeof(Block))
        index++;

    my_printf("Block size %d was shared, new size = %d\n", block->size, block->size / 2);

    Block* block_copy = (Block*)((char*)block + (block->size + sizeof(Block)) / 2);

    if (block->prev != NULL) {
        block->prev->next = block->next;
    }
    else {
        allocator->freeLists[index] = block->next;
    }

    if (block->next != NULL)
        block->next->prev = block->prev;

    index--;
    block->prev = NULL;
    block->next = block_copy;
    block_copy->prev = block;
    block_copy->next = allocator->freeLists[index];

    if (allocator->freeLists[index] != NULL) {
        allocator->freeLists[index]->prev = block_copy;
    }
    allocator->freeLists[index] = block;

    block->size = power(2, index) - sizeof(Block);
    block_copy->size = power(2, index) - sizeof(Block);
}

extern "C" MYLIBRARY_API void* allocator_alloc(Allocator* allocator, size_t size) {
    int index = 0;
    while ((1 << (index + allocator->start_extent)) < size + sizeof(Block)) {
        index++;
    }
    my_printf("Allocation block: size %d, index: %d\n", size, index);

    if (index >= NUM_LISTS) {
        my_printf("Index >= 11\n");
        return NULL;
    }

    if (allocator->freeLists[index] != NULL) {
        Block* block = allocator->freeLists[index];
        allocator->freeLists[index]->prev = NULL;
        allocator->freeLists[index] = block->next;
        my_printf("List index %d has free block size %d offset %d\n", index, block->size, (char*)block - (char*)allocator->memory);
        return block + 1;
    }

    my_printf("List index %d has not free block, try to find block to split\n", index);
    int i = index;
    while (i < NUM_LISTS&& allocator->freeLists[i] == NULL)
        i++;

    if (i == NUM_LISTS) {
        my_printf("Not found\n");
        return NULL;
    }

    my_printf("Found, index %d\n", i);

    for (int j = i; j > index; j--) {
        Block* block = allocator->freeLists[j];
        my_printf("Split block index: %d\n", j);
        split_block(allocator, block);
    }
    my_printf("return my_malloc\n");
    return allocator_alloc(allocator, size);
}

extern "C" MYLIBRARY_API void allocator_free(Allocator* allocator, void* ptr) {

    if (allocator == NULL || ptr == NULL)
        return;

    Block* block = ((Block*)ptr) - 1;

    int index = 0;
    while ((1 << (index + allocator->start_extent)) < block->size + sizeof(Block))
        index++;

    block->prev = NULL;
    if (allocator->freeLists[index] == NULL) {
        block->next = NULL;
    }
    else {
        allocator->freeLists[index]->prev = block;
        block->next = allocator->freeLists[index];
    }
    allocator->freeLists[index] = block;
}

extern "C" MYLIBRARY_API void allocator_destroy(Allocator* allocator) {
    if (!allocator)
        return;
}
