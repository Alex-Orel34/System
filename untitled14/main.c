#include <stdio.h>
#include <windows.h>

int main() {
    LPCSTR pipe_name = "\\\\.\\pipe\\.mypipe";
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    char* message;
    DWORD address;
    BOOL result;

    printf("Creating duplex pipe...\n");
    hPipe = CreateNamedPipeA(pipe_name, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, sizeof(DWORD), sizeof(DWORD), 200, NULL);

    if (hPipe == INVALID_HANDLE_VALUE){
        printf("Pipe wasn't created correctly");

        return 1;
    }

    printf("COnnecting pipe...\n");
    result = ConnectNamedPipe(hPipe, NULL);
    if (result == FALSE){
        printf("Pipe is not connected\n");
        CloseHandle(hPipe);

        return 1;
    }

    result = ReadFile(hPipe, &address, sizeof(address), NULL, NULL);
    if (result == FALSE){
        printf("Pipe is not readable");
        CloseHandle(hPipe);

        return 1;
    }

    printf("Wow! That's user message:\n");
    message = (char*) address;
    printf("%s\n", message);


    printf("Sending answer...\n");
    result = WriteFile(hPipe, &address, sizeof(address),NULL,NULL);
    if (result == FALSE) {
        printf("Answer wasn't sent!\n");
        CloseHandle(hPipe);

        return 1;
    }

    printf("Disconnecting pipe...\n");
    result = DisconnectNamedPipe(hPipe);
    if (result == FALSE) {
        printf("Pipe is not disconnected");
        CloseHandle(hPipe);

        return 1;
    }

    printf("Closing pipe...\n");

    result = CloseHandle(hPipe);
    if (result == FALSE) {
        printf("Pipe couldn't be closed");

        return 1;
    }

    printf("Process B was finished...\n");
    return 0;
}