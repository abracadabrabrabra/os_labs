#define _CRT_SECURE_NO_WARNINGS
#define BUF_SIZE 1000
#define MAX_THREADS 100000
#include "my_stdio.h"
#include <math.h>
#include <time.h>

enum ret_type {
    SUCCESS,
    FILE_OPENING_ERROR,
    ERROR_MALLOC,
    ERROR_ARGS_COUNT,
    ERR_SEM,
    ERR_THREAD
};

unsigned long long to_dec(const char* num, int base){
    unsigned long long res = 0, i = 0;
    while (num[i]) {
        res = res * base + (num[i] <= '9' ? num[i] - '0' : num[i] - 'A' + 10);
        i++;
    }
    return res;
}

unsigned long long sum = 0, cnt = 0;
HANDLE semaphore;

typedef struct {
    const unsigned long long* numbers;
    unsigned long long count;
} ThreadData;

DWORD WINAPI sum_array_part(LPVOID lpParam) {
    ThreadData* data = (ThreadData*)lpParam;
    unsigned long long local_sum = 0;

    for (unsigned long long i = 0; i < data->count; ++i) {
        //my_printf("%l\n", data->numbers[i]);
        local_sum += data->numbers[i];
    }

    WaitForSingleObject(semaphore, INFINITE);
    sum += local_sum;
    cnt += data->count;
    ReleaseSemaphore(semaphore, 1, NULL);

    return 0;
}



int main(int argc, char* argv[]) {

    HANDLE hFile = CreateFile(ConvertToWideString(argv[1]), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        my_printf("Failed to open file");
        return FILE_OPENING_ERROR;
    }

    char buffer[BUF_SIZE];
    DWORD bytesRead;
    ReadFile(hFile, buffer, BUF_SIZE, &bytesRead, NULL);
    buffer[bytesRead] = '\0';
    CloseHandle(hFile);

    unsigned long long cnt_nums = to_dec(argv[2], 10) / sizeof(unsigned long long);
    unsigned long long* nums = (unsigned long long*)malloc(sizeof(unsigned long long) * cnt_nums);
    if (!nums)
        return ERROR_MALLOC;

    int NUM_THREADS = to_dec(argv[3], 10);
    if (!NUM_THREADS) {
        my_printf("No threads error\n");
        return ERR_THREAD;
    }

    if (NUM_THREADS > MAX_THREADS || NUM_THREADS > cnt_nums) {
        my_printf("\nMax threads limit exceeded. Setting to %d\n\n", cnt_nums);
        NUM_THREADS = cnt_nums;
    }

    semaphore = CreateSemaphore(NULL, NUM_THREADS, NUM_THREADS, NULL);
    if (!semaphore) {
        free(nums);
        my_printf("Create semaphore error occured\n");
        return ERR_SEM;
    }

    char* pch = strtok(buffer, "\n");
    unsigned long long i = 0;
    while (pch) {
        nums[i] = to_dec(pch, 16);
        pch = strtok(NULL, "\n");
        i++;
    }

    HANDLE* threads = (HANDLE*)malloc(NUM_THREADS * sizeof(HANDLE));
    if (!threads) {
        free(nums);
        return ERROR_MALLOC;
    }
    ThreadData* thread_data = (ThreadData*)malloc(NUM_THREADS * sizeof(ThreadData));
    if (!thread_data) {
        free(nums);
        free(threads);
        return ERROR_MALLOC;
    }

    unsigned long long size_thread = cnt_nums / NUM_THREADS;
    //my_printf("cnt = %l, nt = %l, st = %l\n", cnt_nums, NUM_THREADS, size_thread);
    clock_t start = clock();

    for (i = 0; i < NUM_THREADS; i++) {
        thread_data[i].numbers = &(nums[i * size_thread]);
        thread_data[i].count = (i * (size_thread + 1) <= cnt_nums && i < NUM_THREADS - 1) ? size_thread : cnt_nums - i * size_thread;
        //my_printf("ind = %l, count = %l\n", i, thread_data[i].count);
        threads[i] = CreateThread(NULL, 0, sum_array_part, (LPVOID)&thread_data[i], 0, NULL);
        if (!threads[i]) {
            free(nums);
            free(threads);
            free(thread_data);
            my_printf("Create thread error occured\n");
            return ERR_THREAD;
        }
    }

    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    my_printf("Time: %k src\n\n", seconds);

    if (!cnt) {
        my_printf("%l %l\n", sum, cnt);
        my_printf("File is empty\n");
        free(nums);
        free(threads);
        free(thread_data);
        return ERROR_ARGS_COUNT;
    }

    my_printf("Sum = %l, count = %l, average value = %l\n", sum, cnt, llround(1. * sum / cnt));

    
    CloseHandle(semaphore);
    for (unsigned long long i = 0; i < NUM_THREADS; ++i)
        CloseHandle(threads[i]);

    free(nums);
    free(threads);
    free(thread_data);
    return 0;
}
