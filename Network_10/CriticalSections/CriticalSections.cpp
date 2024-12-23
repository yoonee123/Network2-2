#include "..\..\Common.h" // 공통 유틸리티 함수(err_quit, err_display 등)를 포함하는 헤더 파일을 포함

#define REMOTEIP   "255.255.255.255" // 브로드캐스트 IP 주소
#define REMOTEPORT 9000              // 브로드캐스트 송신 포트 번호
#define BUFSIZE    512               // 데이터 버퍼 크기
#define LOCALPORT  9000              // 브로드캐스트 수신 포트 번호

// 송신 스레드 함수
DWORD WINAPI SenderThread(LPVOID arg)
{
    int retval;

    // UDP 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()"); // 소켓 생성 실패 시 종료

    // 브로드캐스트 활성화 설정
    DWORD bEnable = 1;
    retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&bEnable, sizeof(bEnable));
    if (retval == SOCKET_ERROR) err_quit("setsockopt()"); // 설정 실패 시 종료

    // 브로드캐스트 대상 주소 구조체 초기화
    struct sockaddr_in remoteaddr;
    memset(&remoteaddr, 0, sizeof(remoteaddr));         // 구조체 메모리 초기화
    remoteaddr.sin_family = AF_INET;                    // 주소 체계 설정 (IPv4)
    inet_pton(AF_INET, REMOTEIP, &remoteaddr.sin_addr); // 문자열 IP 주소를 네트워크 바이트 순서로 변환
    remoteaddr.sin_port = htons(REMOTEPORT);            // 포트 번호를 네트워크 바이트 순서로 설정

    // 데이터 송신에 사용할 변수 선언
    char buf[BUFSIZE + 1]; // 데이터 버퍼
    int len;               // 입력 데이터 길이

    // 브로드캐스트 데이터 송신 루프
    while (1) {
        // 사용자로부터 데이터 입력
        printf("\n[보낼 데이터] ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL) // 입력 실패 시 종료
            break;

        len = (int)strlen(buf);           // 입력된 데이터 길이 계산
        if (buf[len - 1] == '\n')         // 줄바꿈 문자 제거
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)             // 빈 문자열 입력 시 종료
            break;

        // 데이터를 브로드캐스트로 송신
        retval = sendto(sock, buf, (int)strlen(buf), 0, (struct sockaddr*)&remoteaddr, sizeof(remoteaddr));
        if (retval == SOCKET_ERROR) {    // 송신 실패 시 오류 출력 후 종료
            err_display("sendto()");
            break;
        }
    }

    closesocket(sock); // 사용한 소켓 닫기
    return 0;          // 스레드 종료
}

// 수신 스레드 함수
DWORD WINAPI ReceiveThread(LPVOID arg)
{
    int retval;

    // UDP 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()"); // 소켓 생성 실패 시 종료

    // 소켓을 로컬 주소에 바인딩
    struct sockaddr_in localaddr;
    memset(&localaddr, 0, sizeof(localaddr));       // 구조체 메모리 초기화
    localaddr.sin_family = AF_INET;                // 주소 체계 설정 (IPv4)
    localaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 네트워크 인터페이스에서 수신
    localaddr.sin_port = htons(LOCALPORT);         // 로컬 포트 번호 설정
    retval = bind(sock, (struct sockaddr*)&localaddr, sizeof(localaddr));
    if (retval == SOCKET_ERROR) err_quit("bind()"); // 바인딩 실패 시 종료

    // 데이터 수신에 사용할 변수 선언
    struct sockaddr_in peeraddr; // 송신자 주소 저장 구조체
    int addrlen;                 // 송신자 주소 길이
    char buf[BUFSIZE + 1];       // 데이터 버퍼

    // 브로드캐스트 데이터 수신 루프
    while (1) {
        addrlen = sizeof(peeraddr); // 송신자 주소 구조체 크기 설정
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr*)&peeraddr, &addrlen);
        if (retval == SOCKET_ERROR) { // 수신 실패 시 오류 출력 후 종료
            err_display("recvfrom()");
            break;
        }

        buf[retval] = '\0'; // 수신 데이터 끝에 NULL 문자 추가
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &peeraddr.sin_addr, addr, sizeof(addr)); // 송신자 IP 주소를 문자열로 변환
        printf("%s\n", buf); // 수신 데이터 출력
    }

    closesocket(sock); // 사용한 소켓 닫기
    return 0;          // 스레드 종료
}

int main(int argc, char* argv[])
{
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) // 윈속 초기화 실패 시 종료
        return 1;

    // 송신 스레드와 수신 스레드 생성
    HANDLE hThread[2];
    hThread[0] = CreateThread(NULL, 0, SenderThread, NULL, 0, NULL); // 송신 스레드 생성
    hThread[1] = CreateThread(NULL, 0, ReceiveThread, NULL, 0, NULL); // 수신 스레드 생성

    // 두 스레드가 종료될 때까지 대기
    WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

    // 윈속 종료
    WSACleanup();
    return 0; // 프로그램 종료
}
