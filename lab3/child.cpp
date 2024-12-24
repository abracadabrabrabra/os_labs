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
    ERR_FMAP,
    ERR_SEM,
    ERR_MAPVIEW
};

int main() {
    
    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;

    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, ConvertToWideString("SharedMemory"));
    HANDLE hSemParent = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, ConvertToWideString("SemaphoreParent"));
    HANDLE hSemChild = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, ConvertToWideString("SemaphoreChild"));
    if (hMapFile == NULL || hSemParent == NULL || hSemChild == NULL) {
        my_printf("Failed to open file mapping or semaphore\n");
        return ERR_FMAP;
    }

    char* pBuf = (char*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (pBuf == NULL) {
        CloseHandle(hMapFile);
        CloseHandle(hSemParent);
        CloseHandle(hSemChild);
        my_printf("Failed to map view of file\n");
        return ERR_MAPVIEW;
    }

    WaitForSingleObject(hSemChild, INFINITE);
    strcpy(buffer, pBuf);

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
                        strcpy(pBuf, "DIVIDE_BY_ZERO");
                        ReleaseSemaphore(hSemParent, 1, NULL);
                        UnmapViewOfFile(pBuf);
                        CloseHandle(hMapFile);
                        CloseHandle(hSemParent);
                        CloseHandle(hSemChild);
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

    ReleaseSemaphore(hSemParent, 1, NULL);
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    CloseHandle(hSemParent);
    CloseHandle(hSemChild);


    return SUCCESS;
}