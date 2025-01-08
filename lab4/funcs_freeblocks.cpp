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

extern "C" MYLIBRARY_API Allocator* allocator_create(void* memory, const size_t size) {

    Allocator* allocator = (Allocator*)memory;
    if (size < sizeof(Allocator))
        return NULL;

    allocator->memory = (char*)memory + sizeof(Allocator);
    allocator->total_size = size - sizeof(Allocator);

    allocator->blocks = (Block*)allocator->memory;
    allocator->blocks->free_space = allocator->total_size - sizeof(Block);
    allocator->blocks->size = 0;
    allocator->blocks->next = NULL;

    my_printf("Allocator size: %d Block size: %d\n", sizeof(Allocator), sizeof(Block));

    my_printf("Allocator was created, size (without 1 block) = %d\n", allocator->blocks->free_space);
    my_printf("Beginning %d\n", (char*)allocator->memory - (char*)memory);

    return allocator;
}

extern "C" MYLIBRARY_API void allocator_destroy(Allocator* allocator) {
    allocator->blocks = NULL;
    my_printf("Allocator was destroyed\n");
}

extern "C" MYLIBRARY_API void print(Block* block) {
    if (!block) {
        my_printf("Block: NULL\n");
        return;
    }
    my_printf("Block: free: %d size: %d\n", block->free_space, block->size);
}

extern "C" MYLIBRARY_API void print_blocks(Allocator* allocator) {
    my_printf("Blocks:--------------\n");
    Block* cur_block = allocator->blocks;
    while (cur_block) {
        print(cur_block);
        cur_block = cur_block->next;
    }
    my_printf("---------------------\n");
}

extern "C" MYLIBRARY_API void* allocator_alloc(Allocator* allocator, const size_t size) {
    my_printf("Start allocationg, size = %d\n", size);
    Block* cur_block = allocator->blocks;
    Block* prev = NULL;

    size_t size_with_block = size + sizeof(Block);
    my_printf("Size with block: %d\n", size_with_block);

    Block* best_block = NULL;
    while (cur_block) {
        if (cur_block->free_space >= size_with_block && (best_block == NULL || cur_block->free_space < best_block->free_space)) {
            best_block = cur_block;
        }
        prev = cur_block;
        cur_block = cur_block->next;
    }

    my_printf("Best: ");
    print(best_block);

    if (best_block == NULL)
        return NULL;

    if (best_block->size == 0) {
        best_block->size = size;
        best_block->free_space -= size;

        return (void*)((char*)best_block + sizeof(Block));
    }

    if (best_block->free_space > size_with_block + sizeof(Block)) {
        my_printf("Sharing (%d > %d)\n", best_block->free_space, size_with_block + sizeof(Block));

        Block* new_block = (Block*)((char*)best_block + best_block->size + sizeof(Block));

        new_block->free_space = best_block->free_space - size_with_block;
        new_block->next = best_block->next;
        new_block->size = size;

        my_printf("New block: ");
        print(new_block);

        best_block->free_space = 0;

        if (prev)
            prev->next = new_block;
        else
            allocator->blocks = new_block;

        return (void*)((char*)new_block + sizeof(Block));

    }
    else {
        my_printf("Deleting\n");
        best_block->free_space = 0;
        if (prev)
            prev->next = best_block->next;
        else
            allocator->blocks = best_block->next;

        return (void*)((char*)best_block + sizeof(Block));
    }
}

extern "C" MYLIBRARY_API void allocator_free(Allocator* allocator, void* memory) {
    if (!memory)
        return;

    Block* block = (Block*)((char*)memory - sizeof(Block));
    my_printf("Block to free: ");
    print(block);

    block->free_space += block->size;
    block->size = 0;
}
