#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define MAXPENDING 5
#define RECVBUFSIZE 32

void DieWithError(const char *errorMessage) {
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
    exit(1);
}

void HandleTCPClient(SOCKET clntSocket) {
    char echoBuffer[RECVBUFSIZE];
    int recvMsgSize;

    if ((recvMsgSize = recv(clntSocket, echoBuffer, RECVBUFSIZE, 0)) == SOCKET_ERROR) {
        DieWithError("recv() failed");
    }

    while (recvMsgSize > 0) {
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize) {
            DieWithError("send() failed");
        }

        if ((recvMsgSize = recv(clntSocket, echoBuffer, RECVBUFSIZE, 0)) == SOCKET_ERROR) {
            DieWithError("recv() failed");
        }
    }

    closesocket(clntSocket);
}

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    SOCKET servSock, clntSock;
    struct sockaddr_in echoServAddr, echoClntAddr;
    unsigned short echoServPort;
    int clntLen;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        DieWithError("WSAStartup() failed");
    }

    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        DieWithError("socket() failed");
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServAddr.sin_port = htons(echoServPort);

    if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) == SOCKET_ERROR) {
        DieWithError("bind() failed");
    }

    if (listen(servSock, MAXPENDING) == SOCKET_ERROR) {
        DieWithError("listen() failed");
    }

    for (;;) {
        clntLen = sizeof(echoClntAddr);

        if ((clntSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen)) == INVALID_SOCKET) {
            DieWithError("accept() failed");
        }

        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
        HandleTCPClient(clntSock);
    }

    closesocket(servSock);
    WSACleanup();
    return 0;
}