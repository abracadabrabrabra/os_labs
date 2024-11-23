#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string.h>
#include "my_stdio.h"
//#include <stdio.h>
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

int main(int argc, char* argv[]) {

    //my_printf("1\n");
    
    if (argc != 2) {
        my_printf("Incorrect num args");
        return ERROR_ARGS_COUNT;
    }

    HANDLE hPipe1, hPipe2;
    char pipeName1[] = "\\\\.\\pipe\\Pipe1";
    char pipeName2[] = "\\\\.\\pipe\\Pipe2";
    char fileName[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;

    hPipe1 = CreateNamedPipe(ConvertToWideString(pipeName1), PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 0, NULL);
    if (hPipe1 == INVALID_HANDLE_VALUE) {
        my_printf("Failed to create named pipe");
        return ERROR_CREATE_PIPE;
    }

    hPipe2 = CreateNamedPipe(ConvertToWideString(pipeName2), PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 0, NULL);
    if (hPipe2 == INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe1);
        my_printf("Failed to create named pipe");
        return ERROR_CREATE_PIPE;
    }

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
        CloseHandle(hPipe1);
        CloseHandle(hPipe2);
        my_printf("Failed to create process");
        return ERROR_CREATE_CHILD_PROCESS;
    }

    ConnectNamedPipe(hPipe1, NULL);

    ConnectNamedPipe(hPipe2, NULL);
    WriteFile(hPipe1, buffer, bytesRead + 1, &bytesWritten, NULL);
    char response[BUFFER_SIZE] = { '\0' };
    ReadFile(hPipe2, response, BUFFER_SIZE, &bytesRead, NULL);
    if (!strcmp(response, "DIVIDE_BY_ZERO")) {
         my_printf("Zero division error");
         return ERROR_DEV_ZERO;
    }


    CloseHandle(hPipe1);
    CloseHandle(hPipe2);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return SUCCESS;
}
