#include <windows.h>
#include <time.h>
//#include <iostream>
#include "C:/Users/T470/source/repos/DLL_FREEBLOCKS/funcs.h"  

typedef int(*AddFunc)(int, int);
typedef Allocator*(*func_create)(void*, const size_t);
typedef void(*func_my_printf)(const char*, ...);
typedef void(*func_destroy)(Allocator*);
typedef void(*func_print)(Block*);
typedef void(*func_print_blocks)(Allocator*);
typedef void*(*func_alloc)(Allocator *, const size_t);
typedef void(*func_free)(Allocator *, void*);

void my_print(const char* format, ...) {
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

LPWSTR ConvertToWideString(const char* str) {
    if (str == nullptr) return nullptr;
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    LPWSTR wideString = new wchar_t[size_needed];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wideString, size_needed);
    return wideString;
}

Allocator* system_allocator_create(void* memory, size_t size) {
    my_print("System allocator was created\n");
    return (Allocator*)memory;
}

void system_allocator_destroy(Allocator* allocator) {
    my_print("System allocator was destroyed\n");
    // Ничего не делаем для системного аллокатора
}

void* system_allocator_alloc(Allocator* allocator, size_t size) {
    my_print("System alloc %d bytes\n", size);
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void system_allocator_free(Allocator* allocator, void* memory) {
    my_print("System free\n");
    VirtualFree(memory, 0, MEM_RELEASE);
}

void system_print_blocks(Allocator* A) {
    my_print("System print blocks\n");
}

//path 1: C:/Users/T470/source/repos/DLL_FREEBLOCKS/x64/Debug/DLL_FREEBLOCKS.dll
//path 2: C:/Users/T470/source/repos/Dll_POW2/x64/Debug/Dll_POW2.dll

int main(int argc, char* argv[]) {
    if (argc > 2) {
        my_print("Incorrect number of arguments");
        return 3;
    }
    if (argc == 2) {
        HMODULE hModule = LoadLibrary(ConvertToWideString(argv[1]));
        if (hModule) {
            func_print_blocks p_blocks = (func_print_blocks)GetProcAddress(hModule, "print_blocks");
            if (!p_blocks) {
                my_print("Could not locate the print_blocks function.");
                FreeLibrary(hModule);
                return 1;
            }
            func_create create = (func_create)GetProcAddress(hModule, "allocator_create");
            if (!create) {
                my_print("Could not locate the create function.");
                FreeLibrary(hModule);
                return 1;
            }
            func_alloc alloc = (func_alloc)GetProcAddress(hModule, "allocator_alloc");
            if (!alloc) {
                my_print("Could not locate the alloc function.");
                FreeLibrary(hModule);
                return 1;
            }
            func_free free_func = (func_free)GetProcAddress(hModule, "allocator_free");
            if (!free_func) {
                my_print("Could not locate the free function.");
                FreeLibrary(hModule);
                return 1;
            }
            func_destroy destroy = (func_destroy)GetProcAddress(hModule, "allocator_destroy");
            if (!destroy) {
                my_print("Could not locate the destroy function.");
                FreeLibrary(hModule);
                return 1;
            }

            //char data[MAX_SIZE];
            void* data = VirtualAlloc(NULL, MAX_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (!data) {
                my_print("Virtual alloc error");
                FreeLibrary(hModule);
                return 2;
            }
            Allocator* a = create(data, MAX_SIZE);
            p_blocks(a);

            int sz[13] = { 10, 20, 100, 150, 1024, 80, 35, 32, 55, 2048, 5000, 136, 25 };
            char* pc[13];
            for (int i = 0; i < 13; i++) {
                clock_t start = clock();
                pc[i] = (char*)alloc(a, sz[i]);
                clock_t end = clock();
                if (!pc[i]) {
                    my_print("Allocation error");
                }
                my_print("Time = %k\n", (double)(end - start) / CLOCKS_PER_SEC);
                p_blocks(a);
            }

            my_print("\n");

            for (int i = 0; i < 13; i++) {
                clock_t start = clock();
                free_func(a, (void*)pc[i]);
                clock_t end = clock();
                my_print("Time = %k\n", (double)(end - start) / CLOCKS_PER_SEC);
                p_blocks(a);
            }

            destroy(a);
            VirtualFree(data, 0, MEM_RELEASE);
            FreeLibrary(hModule);
        }
    }
    else {
        my_print("Could not load the DLL, using system funcs.\n\n");
        func_print_blocks p_blocks = (func_print_blocks)system_print_blocks;
        func_create create = (func_create)system_allocator_create;
        func_alloc alloc = (func_alloc)system_allocator_alloc;
        func_free free_func = (func_free)system_allocator_free;
        func_destroy destroy = (func_destroy)system_allocator_destroy;
        
        void* data = VirtualAlloc(NULL, MAX_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!data) {
            my_print("Virtual alloc error");
            return 2;
        }

        Allocator* a = create(data, MAX_SIZE);
        p_blocks(a);

        
        int sz[13] = { 10, 20, 100, 150, 1024, 80, 35, 32, 55, 2048, 5000, 136, 25 };
        char* pc[13];
        for (int i = 0; i < 13; i++) {
            clock_t start = clock();
            pc[i] = (char*)alloc(a, sz[i]);
            clock_t end = clock();
            if (!pc[i]) {
                my_print("Allocation error");
            }
            my_print("Time = %k\n", (double)(end - start) / CLOCKS_PER_SEC);
            p_blocks(a);
        }

        my_print("\n");

        for (int i = 0; i < 13; i++) {
            clock_t start = clock();
            free_func(a, (void*)pc[i]);
            clock_t end = clock();
            my_print("Time = %k\n", (double)(end - start) / CLOCKS_PER_SEC);
            p_blocks(a);
        }

        destroy(a);
        VirtualFree(data, 0, MEM_RELEASE);
    }
    return 0;
}
