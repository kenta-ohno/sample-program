import socket
import sys

RCVBUFSIZE = 64

def DieWithError(errorMessage):
    print(errorMessage, file=sys.stderr)
    sys.exit(1)

def main():
    if len(sys.argv) != 2:
        sys.exit(1)

    servIP = "127.0.0.1"
    echoString = sys.argv[1]
    echoServerPort = 7  # エコーサービスのwell-knownポート

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except socket.error as e:
        DieWithError(f"socket() failed: {e}")

    echoServerAddr = (servIP, echoServerPort)

    try:
        sock.connect(echoServerAddr)
    except socket.error as e:
        DieWithError(f"connect() failed: {e}")

    echoStringLen = len(echoString)

    try:
        if sock.send(echoString.encode()) != echoStringLen:
            DieWithError("send() sent a different number of bytes than expected")
    except socket.error as e:
        DieWithError(f"send() failed: {e}")

    totalBytesRcvd = 0
    print("Received: ", end='')

    while totalBytesRcvd < echoStringLen:
        try:
            bytesRcvd = sock.recv(RCVBUFSIZE - 1)
            if not bytesRcvd:
                DieWithError("recv() failed")
            totalBytesRcvd += len(bytesRcvd)
            print(bytesRcvd.decode(), end='')
        except socket.error as e:
            DieWithError(f"recv() failed: {e}")

    print()

    sock.close()

if __name__ == "__main__":
    main()