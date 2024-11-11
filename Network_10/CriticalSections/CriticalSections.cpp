#include "..\..\Common.h"

#define REMOTEIP   "255.255.255.255"
#define REMOTEPORT 9000
#define BUFSIZE    512
#define LOCALPORT  9000

DWORD WINAPI SenderThread(LPVOID arg)
{
    int retval;

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // 브로드캐스팅 활성화
    DWORD bEnable = 1;
    retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&bEnable, sizeof(bEnable));
    if (retval == SOCKET_ERROR) err_quit("setsockopt()");

    // 소켓 주소 구조체 초기화
    struct sockaddr_in remoteaddr;
    memset(&remoteaddr, 0, sizeof(remoteaddr));
    remoteaddr.sin_family = AF_INET;
    inet_pton(AF_INET, REMOTEIP, &remoteaddr.sin_addr);
    remoteaddr.sin_port = htons(REMOTEPORT);

    // 데이터 통신에 사용할 변수
    char buf[BUFSIZE + 1];
    int len;

    // 브로드캐스트 데이터 보내기
    while (1) {
        // 데이터 입력
        printf("\n[보낼 데이터] ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
            break;

        len = (int)strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        retval = sendto(sock, buf, (int)strlen(buf), 0, (struct sockaddr*)&remoteaddr, sizeof(remoteaddr));
        if (retval == SOCKET_ERROR) {
            err_display("sendto()");
            break;
        }
    }

    closesocket(sock); // 소켓 닫기
    return 0;
}

DWORD WINAPI ReceiveThread(LPVOID arg)
{
    int retval;

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    struct sockaddr_in localaddr;
    memset(&localaddr, 0, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localaddr.sin_port = htons(LOCALPORT);
    retval = bind(sock, (struct sockaddr*)&localaddr, sizeof(localaddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // 데이터 통신에 사용할 변수
    struct sockaddr_in peeraddr;
    int addrlen;
    char buf[BUFSIZE + 1];

    // 브로드캐스트 데이터 받기
    while (1) {
        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr*)&peeraddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            err_display("recvfrom()");
            break;
        }

        buf[retval] = '\0';
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &peeraddr.sin_addr, addr, sizeof(addr));
        printf("%s\n", buf);
    }

    closesocket(sock); // 소켓 닫기
    return 0;
}

int main(int argc, char* argv[])
{
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // 스레드 두 개 생성
    HANDLE hThread[2];
    hThread[0] = CreateThread(NULL, 0, SenderThread, NULL, 0, NULL);
    hThread[1] = CreateThread(NULL, 0, ReceiveThread, NULL, 0, NULL);

    // 스레드 두 개 종료 대기
    WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

    // 윈속 종료
    WSACleanup();
    return 0;
}
