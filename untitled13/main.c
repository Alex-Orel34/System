#include <stdio.h>
#include <windows.h>
#include <TlHelp32.h>

#define PROCESS_B_NAME "untitled14.exe"
#define MAX_INPUT_LENGTH 100

DWORD GetProcessIdByName(char* process){
    HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (processSnapshot == INVALID_HANDLE_VALUE){
        printf("Can't create processes snapshot\n");

        return -1;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(processSnapshot, &entry)){
        printf("Snapshot is empty\n");
        CloseHandle(processSnapshot);

        return -1;
    }

    do{
        if (strcmp(entry.szExeFile, process) == 0){
            CloseHandle(processSnapshot);

            return entry.th32ProcessID;
        }
    }while(Process32Next(processSnapshot, &entry));

    CloseHandle(processSnapshot);
    printf("Process not found\n");

    return -1;
}


int main() {
    DWORD buffer;
    LPVOID address;
    LPCSTR pipe_name = "\\\\.\\pipe\\.mypipe";
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    printf("Enter a string you want to send\n");
    size_t size = MAX_INPUT_LENGTH * sizeof(char);
    char* input = (char*)malloc(size);
    gets_s(input, MAX_INPUT_LENGTH - 1);
    if (strcmp(input, "") == 0){
        printf("Message can't be empty\n");

        return 1;
    }

    printf("Trying to open process B...\n");
    DWORD related_process_id = GetProcessIdByName(PROCESS_B_NAME);
    if (related_process_id == -1){

        return 1;
    }

    HANDLE processB = OpenProcess(PROCESS_ALL_ACCESS, FALSE, related_process_id);
    if (processB == INVALID_HANDLE_VALUE){
        printf("Process B can't be opened correctly");

        return 1;
    }
    printf("Process B has been opened successfully\n");

    printf("Allocating memory for your message...\n");

    address = VirtualAllocEx(processB, NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    printf("Saving message...\n");


    BOOL result = WriteProcessMemory(processB, address, input, size, NULL);

    if (result == FALSE){
        printf("Saving message has been failed");
        CloseHandle(processB);

        return 1;
    }
    printf("Message has been written successfully\n");

    while(hPipe == INVALID_HANDLE_VALUE){
        hPipe = CreateFileA(pipe_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    }
    printf("Named pipe is connected;\n");
    printf("Writing memory address...\n");
    buffer = (DWORD)address;
    result = WriteFile(hPipe, &buffer, sizeof(buffer), NULL, NULL);

    if (result == FALSE){
        printf("Writing address has been failed");
        CloseHandle(processB);

        return 1;
    }

    printf("Waiting for process B...\n");

    result = ReadFile(hPipe, &address, sizeof(address), NULL, NULL);

    if (result == FALSE){
        printf("Process B is dead :(");
        CloseHandle(processB);

        return 1;
    }

    printf("Releasing process B memory...\n");
    result = VirtualFreeEx(processB, address, 0, MEM_RELEASE);

    if (result == FALSE) {
        printf("Memory couldn't be releasing");
        CloseHandle(processB);

        return 1;
    }

    printf("Closing pipe...\n");
    result = CloseHandle(hPipe);

    if (result == FALSE) {
        printf("Pipe can't be closed correctly");
        CloseHandle(processB);

        return 1;
    }

    printf("Closing process B...\n");
    result = CloseHandle(processB);

    if (result == FALSE) {
        printf("Process B can't be closed correctly");

        return 1;
    }

    printf("Process A finished successfully...\n");
    return 0;
}