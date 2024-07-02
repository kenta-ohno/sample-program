#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXPENDING 5    //同時にキュー可能な接続要求の最大数
#define RECVBUFSIZE 64  //受信バッファのサイズ

void DieWithError(char* errroMessage);  //エラー処理関数
void TCPClientProcess(int clntSocket);   //TCPクライアント処理関数

int main(int argc, char* argv[])
{
    int servSock;                       //サーバソケット
    int clntSock;                       //クライアントソケット
    struct sockaddr_in echoServerAddr;    //ローカルアドレス
    struct sockaddr_in echoClientAddr;    //クライアントアドレス
    unsigned short echoServPort = 7;      //サーバーポート
    unsigned int clntLen;               //クライアントのアドレス構造体の長さ

    //着信接続用のソケット作成
    if((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        DieWithError("socket() failed");
    }

    //ローカルのアドレス構造体を作成
    memset(&echoServerAddr, 0, sizeof(echoServerAddr));
    echoServerAddr.sin_family = AF_INET;
    echoServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServerAddr.sin_port = htons(echoServPort);

    //ローカルアドレスへバインド
    if(bind(servSock, (struct sockaddr*) &echoServerAddr, sizeof(echoServerAddr)) < 0){
        DieWithError("bind() failed");
    }

    //ソケットをリッスンする
    if(listen(servSock, MAXPENDING) < 0){
        DieWithError("listen() failed");
    }

    for (;;)
    {
        /* 入出力パラメータのサイズをセット */
        clntLen = sizeof(echoClientAddr);

        /* クライアントからの接続要求を待機 */
        if((clntSock = accept(servSock, (struct sockaddr *)&echoClientAddr, &clntLen)) < 0){
            DieWithError("accept() failed");
        }

        printf("Handle Client %s\n", inet_ntoa(echoClientAddr.sin_addr));

        TCPClientProcess(clntSock);
    }

    return 1;
}

void TCPClientProcess(int clntSocket)
{
    char echoBuffer[RECVBUFSIZE];
    int recvMsgSize;

    //クライアントからの受信メッセージ
    if((recvMsgSize = recv(clntSocket, echoBuffer, RECVBUFSIZE, 0)) < 0){
        DieWithError("recv() failed");
    }

    while (recvMsgSize > 0)
    {
        //受信した文字列を送信し、転送が終了していなければ次を受信する
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize){
            DieWithError("send() failed");
        }

        //受信するデータが残っていないか確認
        if((recvMsgSize = recv(clntSocket, echoBuffer, RECVBUFSIZE, 0)) < 0){
            DieWithError("recv() failed");
        }
    }
    close(clntSocket);
}

void DieWithError(char* errorMessage)
{
    perror(errorMessage);
    exit(1);
}