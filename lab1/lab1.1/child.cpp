#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string.h>
//#include <stdio.h>
#include "my_stdio.h"

#define BUFFER_SIZE 1024

enum ret_type_t {
    SUCCESS,    //Successful end
    ERROR_ARGS_COUNT,    //Wrong args number
    ERROR_CREATE_PIPE,  //Failed to create a new pipeline
    ERROR_CREATE_CHILD_PROCESS, //Failed to create a child process
    ERROR_READ, //Failed to read from pipe
    ERROR_DEV_ZERO, //Devision by zero detected
    ERROR_FULL, //Overflow
    ERROR_OPEN_FILE,    //Error with file opening
    ERROR_CLOSE_FILE,   //Error with closing file
    ERROR_FILE_WRITE,   //Error with file writing
    ERROR_HANDLER_INHERITED, //Error handler reading
    ERROR_PIPE_WRITE,   //Failed to write smth in the pipe
    ERROR_HEAP,         //Failed to malloc
};

int main() {
    HANDLE hPipe1, hPipe2;
    char pipeName1[] = "\\\\.\\pipe\\Pipe1";
    char pipeName2[] = "\\\\.\\pipe\\Pipe2";
    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;

    //my_printf("11\n");
    HANDLE hFile;

    hPipe1 = CreateFile(ConvertToWideString(pipeName1), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe1 == INVALID_HANDLE_VALUE) {
        my_printf("Failed to create named pipe");
        return ERROR_CREATE_PIPE;
    }
    //my_printf("22\n");
    hPipe2 = CreateFile(ConvertToWideString(pipeName2), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe2 == INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe1);
        my_printf("Failed to create named pipe");
        return ERROR_CREATE_PIPE;
    }
    //my_printf("33\n");
    ReadFile(hPipe1, buffer, BUFFER_SIZE, &bytesRead, NULL);
    //my_printf("44\n");
    //my_printf("%s\n\n", buffer);
    int count = 0;
    char* token = strtok(buffer, "\n");
    //my_printf("start token: %s\n", token);
    while (token) {
        int i = 0, a, b, is_first = 1;
        char num[BUFFER_SIZE];
        char* ptc = token;
        while (1) {
            if (*ptc == ' ' || !(*ptc)) {
                if (is_first) {
                    num[i] = '\0';
                    a = atoi(num);
                    is_first = 0;
                }
                else {
                    num[i] = '\0';
                    b = atoi(num);
                    if (!b) {
                        WriteFile(hPipe2, "DIVIDE_BY_ZERO", strlen("DIVIDE_BY_ZERO"), &bytesWritten, NULL);
                        return ERROR_DEV_ZERO;
                    }
                    my_printf("%d ", a / b);
                }
                i = 0;
                if (!(*ptc))
                    break;
            }
            else {
                num[i] = *ptc;
                i++;
            }
            ptc++;
        }
        my_printf("\n");
        token = strtok(NULL, "\n");
        //my_printf("token: %s\n", token);
    }

    CloseHandle(hPipe1);
    CloseHandle(hPipe2);

    return SUCCESS;
}