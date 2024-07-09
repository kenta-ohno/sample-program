#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RECV_SIZE 32 * 1024
#define SEND_SIZE 32 * 1024

void DieWithError(char* errroMessage);  //エラー処理関数
static char recv_buffer[RECV_SIZE];

typedef struct send_header{
    uint8_t id;
    uint8_t type;
    uint16_t size;
    char reserve[16];
} send_header_t;

static int send_header_payload(int sd, struct sockaddr_in* dest_addr, socklen_t dest_size, char* header, size_t header_size, char* payload, size_t payload_size)
{
    struct msghdr msg;
    struct iovec iov[2];

    //文字列をサーバに送信
    iov[0].iov_base = header;
    iov[0].iov_len = header_size;
    iov[1].iov_base = payload;
    iov[1].iov_len = payload_size;

    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;
    msg.msg_name = dest_addr;
    msg.msg_namelen = dest_size;

    return sendmsg(sd, &msg, 0);
}

static int recv_header_payload(int sd, struct sockaddr_in* cln_add, socklen_t cln_size, char* header, size_t header_size, char* payload, size_t payload_size){

    struct msghdr msg;
    struct iovec iov[2];

    //文字列をサーバに送信
    iov[0].iov_base = header;
    iov[0].iov_len = header_size;
    iov[1].iov_base = payload;
    iov[1].iov_len = payload_size;

    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;
    msg.msg_name = cln_add;
    msg.msg_namelen = cln_size;
    
    //return recv(sd, payload, payload_size, 0);
    return recvmsg(sd, &msg, 0);
}

int main(int argc, char* argv[])
{
    int sock;
    struct sockaddr_in echoServAddr;    /* 送信先のアドレス */
    struct sockaddr_in fromAddr;        /* 送信元のアドレス */
    unsigned short echoServPort;        /* サーバのポート番号 */
    char* toIP;
    char send_data[SEND_SIZE] = {0xAA};
    unsigned int bytesRcvd; /* 受信した応答の長さ */
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    send_header_t header;

    /* 引数チェック */
    if((argc < 2) || (argc > 3)){
        fprintf(stderr, "Usage of %s:<Target IP> <Target Port>", argv[0]);
        exit(1);
    }

    toIP = argv[1];       /* IPアドレスの設定 */

    if(argc == 3){
        echoServPort = atoi(argv[2]);
    }else{
        echoServPort = 7;   //7はエコーサービスのwell-knownポート
    }

    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        DieWithError("socket() failed");
    }

    //ローカルのアドレス構造体を作成
    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServAddr.sin_port = htons(echoServPort);
    //ローカルアドレスへバインド
    if(bind(sock, (struct sockaddr*) &echoServAddr, sizeof(echoServAddr)) < 0){
        DieWithError("bind() failed");
    }

    socklen_t recvBufferSize = RECV_SIZE*2;
    if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvBufferSize, sizeof(recvBufferSize)) < 0){
        DieWithError("set recv buffer() failed");
    }

    socklen_t sendBufferSize = SEND_SIZE + 6;
    if(setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, sizeof(sendBufferSize)) < 0){
        DieWithError("set send buffer() failed");
    }

    socklen_t sockrecvOpt = sizeof(recvBufferSize);
    if(getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvBufferSize, &sockrecvOpt) < 0){
        DieWithError("get recv buffer() failed");
    }
    printf("recv buffer size: %d\n", recvBufferSize);

    socklen_t socksendOpt = sizeof(sendBufferSize);
    if(getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, &socksendOpt) < 0){
        DieWithError("get send buffer() failed");
    }
    printf("send buffer size: %d\n", sendBufferSize);

    for (;;)
    {
        //応答をサーバから受信
        int rn = 0;
        rn = recv_header_payload(sock, &clientAddr, clientAddrLen, (char*)&header, 5, recv_buffer, SEND_SIZE);
        printf("Received: %d\n", rn);
        
        if(rn < 0){
            DieWithError("recv() failed");
        }

        printf("Handle Client %s %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        /* サーバーへ送信 */

        /* ヘッダーに情報格納 */
        header.id = 0x01;
        header.type = 0x02;
        header.size = SEND_SIZE;
        header.reserve[0] = 0xFF;
        header.reserve[1] = 0xAA;
    
        int sn = 0;
        sn = send_header_payload(sock, &clientAddr, clientAddrLen, (char*)&header, 6, send_data, SEND_SIZE);
        printf("Sended: %d\n", sn);
        if(sn != 6 + SEND_SIZE){
            DieWithError("send() sent a different number of bytes than expected");
        }

        printf("\n");
    }

    close(sock);
    exit(0);
}


void DieWithError(char* errorMessage)
{
    perror(errorMessage);
    exit(1);
}