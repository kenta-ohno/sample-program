#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define RCVBUFSIZE 32

void DieWithError(char* errorMessage);

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in echoServAddr;
    unsigned short echoServPort;
    char* servIP;
    char* echoString;
    char echoBuffer[RCVBUFSIZE];
    unsigned int echoStringLen;
    int bytesRcvd, totalBytesRcvd;

    if ((argc < 3) || (argc > 4)) {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];
    echoString = argv[2];

    if (argc == 4) {
        echoServPort = atoi(argv[3]);
    } else {
        echoServPort = 7; // 7 is the well-known port for the echo service
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        DieWithError("WSAStartup() failed");
    }

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        DieWithError("socket() failed");
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);
    echoServAddr.sin_port = htons(echoServPort);

    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) == SOCKET_ERROR) {
        DieWithError("connect() failed");
    }

    echoStringLen = strlen(echoString);

    if (send(sock, echoString, echoStringLen, 0) != echoStringLen) {
        DieWithError("send() sent a different number of bytes than expected");
    }

    totalBytesRcvd = 0;
    printf("Received: ");

    while (totalBytesRcvd < echoStringLen) {
        if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) == SOCKET_ERROR) {
            DieWithError("recv() failed");
        }
        totalBytesRcvd += bytesRcvd;
        echoBuffer[bytesRcvd] = '\0';
        printf("%s", echoBuffer);
    }

    printf("\n");

    closesocket(sock);
    WSACleanup();
    return 0;
}

void DieWithError(char* errorMessage) {
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
    exit(1);
}