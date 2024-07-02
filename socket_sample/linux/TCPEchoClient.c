#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RCVBUFSIZE 64

void DieWithError(char* errroMessage);  //エラー処理関数

int main(int argc, char* argv[])
{
    int sock;
    struct sockaddr_in echoServerAddr;
    unsigned short echoServerPort = 7;  //7はエコーサービスのwell-knownポート
    char* servIP;
    char* echoString;
    char echoBuffer[RCVBUFSIZE];
    unsigned int echoStringLen;
    int bytesRcvd, totalBytesRcvd;

    if(argc != 1){
        exit(1);
    }

    servIP = "127.0.0.1";
    echoString = argv[1];

    if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        DieWithError("socket() failed");
    }

    /* サーバーのアドレス構造体 */
    memset(&echoServerAddr, 0, sizeof(echoServerAddr));
    echoServerAddr.sin_family = AF_INET;
    echoServerAddr.sin_addr.s_addr = inet_addr(servIP);
    echoServerAddr.sin_port = htons(echoServerPort);

    //エコーサーバへの接続
    if(connect(sock, (struct sockaddr *)&echoServerAddr, sizeof(echoServerAddr)) < 0){
        DieWithError("connct() failed");
    }

    echoStringLen = strlen(echoString);

    //文字列をサーバに送信
    if(send(sock, echoString, echoStringLen, 0) != echoStringLen){
        DieWithError("send() sent a different number of bytes than expected");
    }

    //同じ文字列をサーバから受信
    totalBytesRcvd = 0;
    printf("Received: ");

    while (totalBytesRcvd < echoStringLen)
    {
        // バッファサイズに達するまで（NULL文字用の１バイトを除く）
        if((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) < 0){
            DieWithError("recv() failed");
        }
        totalBytesRcvd += bytesRcvd;
        echoBuffer[bytesRcvd] = '\0';
        printf("%s", echoBuffer);
    }

    printf("\n");

    close(sock);
    exit(0);
}


void DieWithError(char* errorMessage)
{
    perror(errorMessage);
    exit(1);
}