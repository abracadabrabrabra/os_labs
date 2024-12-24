#pragma once
#define INITIAL_BUFFER_SIZE 128

#include <windows.h>
#include <stdarg.h>
#include <string.h>


LPWSTR ConvertToWideString(const char* str);

void my_printf(const char* format, ...);
    
int file_printf(HANDLE fileHandle, const char* format, ...);