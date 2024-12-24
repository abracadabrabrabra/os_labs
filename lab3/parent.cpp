#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string.h>
#include "my_stdio.h"
//#include <stdio.h>
#define BUFFER_SIZE 1024

enum ret_type_t {
    SUCCESS,    
    ERROR_ARGS_COUNT,    
    ERROR_CREATE_PIPE,  
    ERROR_CREATE_CHILD_PROCESS, 
    ERROR_READ, 
    ERROR_DEV_ZERO, 
    ERROR_FULL, 
    ERROR_OPEN_FILE,   
    ERROR_CLOSE_FILE,   
    ERROR_FILE_WRITE,   
    ERROR_HANDLER_INHERITED, 
    ERROR_PIPE_WRITE,   
    ERROR_HEAP,
    ERR_FMAP,
    ERR_SEM,
    ERR_MAPVIEW
};

int main(int argc, char* argv[]) {

    //my_printf("1\n");
    
    if (argc != 2) {
        my_printf("Incorrect num args");
        return ERROR_ARGS_COUNT;
    }

    HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUFFER_SIZE, ConvertToWideString("SharedMemory"));
    if (hMapFile == NULL) {
        my_printf("Failed to create file mapping\n");
        return ERR_FMAP;
    }
    HANDLE hSemParent = CreateSemaphore(NULL, 1, 1, ConvertToWideString("SemaphoreParent"));
    HANDLE hSemChild = CreateSemaphore(NULL, 0, 1, ConvertToWideString("SemaphoreChild"));

    if (hSemParent == NULL || hSemChild == NULL) {
        CloseHandle(hMapFile);
        my_printf("Failed to create semaphore\n");
        return ERR_SEM;
    }

    char* pBuf = (char*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (pBuf == NULL) {
        CloseHandle(hMapFile);
        CloseHandle(hSemParent);
        CloseHandle(hSemChild);
        my_printf("Failed to map view of file\n");
        return ERR_MAPVIEW;
    }
    char fileName[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;

    //my_printf("2\n");

    HANDLE hFile = CreateFile(ConvertToWideString(argv[1]), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    ReadFile(hFile, buffer, BUFFER_SIZE, &bytesRead, NULL);
    buffer[bytesRead] = '\0';
    //my_printf("bytes read %d\n", bytesRead);
    //my_printf("%s", buffer);
    
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char cmdLine[] = "child.exe";

    if (!CreateProcess(NULL, ConvertToWideString(cmdLine), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        CloseHandle(hSemParent);
        CloseHandle(hSemChild);
        my_printf("Failed to create process");
        return ERROR_CREATE_CHILD_PROCESS;
    }

    WaitForSingleObject(hSemParent, INFINITE);
    strcpy(pBuf, buffer);
    ReleaseSemaphore(hSemChild, 1, NULL);

    WaitForSingleObject(hSemParent, INFINITE);
    if (strcmp(pBuf, "DIVIDE_BY_ZERO") == 0) {
        my_printf("Zero division error");
        return ERROR_DEV_ZERO;
    }

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    CloseHandle(hSemParent);
    CloseHandle(hSemChild);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return SUCCESS;
}