import socket
import sys

MAXPENDING = 5    # 同時にキュー可能な接続要求の最大数
RECVBUFSIZE = 64  # 受信バッファのサイズ

def DieWithError(errorMessage):
    print(errorMessage)
    sys.exit(1)

def TCPClientProcess(clntSocket):
    echoBuffer = bytearray(RECVBUFSIZE)
    
    try:
        recvMsgSize = clntSocket.recv_into(echoBuffer)
    except socket.error as e:
        DieWithError(f"recv() failed: {e}")

    while recvMsgSize > 0:
        try:
            if clntSocket.send(echoBuffer[:recvMsgSize]) != recvMsgSize:
                DieWithError("send() failed")
        except socket.error as e:
            DieWithError(f"send() failed: {e}")

        try:
            recvMsgSize = clntSocket.recv_into(echoBuffer)
        except socket.error as e:
            DieWithError(f"recv() failed: {e}")

    clntSocket.close()

def main():
    echoServPort = 7  # サーバーポート

    try:
        servSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except socket.error as e:
        DieWithError(f"socket() failed: {e}")

    echoServerAddr = ('', echoServPort)

    try:
        servSock.bind(echoServerAddr)
    except socket.error as e:
        DieWithError(f"bind() failed: {e}")

    try:
        servSock.listen(MAXPENDING)
    except socket.error as e:
        DieWithError(f"listen() failed: {e}")

    while True:
        try:
            clntSock, echoClientAddr = servSock.accept()
        except socket.error as e:
            DieWithError(f"accept() failed: {e}")

        print(f"Handle Client {echoClientAddr[0]}")
        TCPClientProcess(clntSock)

if __name__ == "__main__":
    main()