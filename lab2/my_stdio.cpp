#pragma once
#define INITIAL_BUFFER_SIZE 128

#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

LPWSTR ConvertToWideString(const char* str) {
    if (str == nullptr) return nullptr;
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    LPWSTR wideString = new wchar_t[size_needed];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wideString, size_needed);
    return wideString;
}

void my_printf(const char* format, ...) {
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


int file_printf(HANDLE fileHandle, const char* format, ...) {
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
            case 's': {
                const char* str = va_arg(args, const char*);
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
    DWORD bytesWritten;
    WriteFile(fileHandle, buffer, (DWORD)(buf_ptr - buffer), &bytesWritten, NULL);

    va_end(args);
    //CloseHandle(fileHandle);
    return 0;
}