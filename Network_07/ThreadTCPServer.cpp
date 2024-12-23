#include "..\..\Common.h"  // 공통 헤더 파일 포함 (에러 처리 함수나 공통 설정이 있을 수 있음)

#define SERVERPORT 9000  // 서버 포트 번호 (9000번 포트)
#define BUFSIZE    512   // 버퍼 크기 (최대 512바이트)

// 클라이언트와 데이터 통신을 처리할 스레드 함수
DWORD WINAPI ProcessClient(LPVOID arg)
{
    int retval;  // 반환 값 저장 변수 (함수 호출 결과 체크)
    SOCKET client_sock = (SOCKET)arg;  // 클라이언트 소켓
    struct sockaddr_in clientaddr;  // 클라이언트 주소 정보 저장 구조체
    char addr[INET_ADDRSTRLEN];  // 클라이언트의 IP 주소를 문자열로 저장할 변수
    int addrlen;  // 클라이언트 주소 길이
    char buf[BUFSIZE + 1];  // 데이터 수신 및 송신 버퍼
    int num[2];  // 클라이언트로부터 받은 두 정수를 저장할 배열
    int sum;  // 두 정수의 합을 저장할 변수

    // 클라이언트 정보 얻기
    addrlen = sizeof(clientaddr);  // 주소 구조체의 크기
    getpeername(client_sock, (struct sockaddr *)&clientaddr, &addrlen);  // 클라이언트 주소 가져오기
    inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));  // IP 주소를 문자열로 변환

    while (1) {
        // 데이터 받기
        retval = recv(client_sock, (char*)num, sizeof(num), 0);  // 클라이언트로부터 두 정수 받기
        if (retval == SOCKET_ERROR) {  // 오류 발생 시
            err_display("recv()");
            break;
        }
        else if (retval == 0)  // 클라이언트가 연결을 종료했을 경우
            break;

        // 받은 데이터 출력
        printf("[TCP/%s:%d] %d %d\n", addr, ntohs(clientaddr.sin_port), num[0], num[1]);

        sum = num[0] + num[1];  // 두 정수의 합 계산

        // 합계 출력
        printf("[TCP/%s:%d] 합계 : %d\n", addr, ntohs(clientaddr.sin_port), sum);

        // 데이터 보내기 (합계 전송)
        retval = send(client_sock, (const char*)&sum, sizeof(sum), 0);  // 계산된 합계를 클라이언트에게 전송
        if (retval == SOCKET_ERROR) {  // 오류 발생 시
            err_display("send()");
            break;
        }
    }

    // 소켓 닫기
    closesocket(client_sock);
    printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
        addr, ntohs(clientaddr.sin_port));
    return 0;
}

int main(int argc, char *argv[])
{
    int retval;  // 반환 값 저장 변수

    // 윈속 초기화
    WSADATA wsa;  // 윈속 관련 데이터 구조체
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)  // 윈속 초기화 (버전 2.2)
        return 1;  // 초기화 실패 시 1을 반환하고 종료

    // 소켓 생성
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);  // TCP 소켓 생성
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");  // 소켓 생성 실패 시 종료

    // bind()
    struct sockaddr_in serveraddr;  // 서버 주소 구조체
    memset(&serveraddr, 0, sizeof(serveraddr));  // 구조체 초기화
    serveraddr.sin_family = AF_INET;  // 주소 체계: IPv4
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  // 모든 네트워크 인터페이스에서 연결을 받도록 설정
    serveraddr.sin_port = htons(SERVERPORT);  // 포트 번호 설정 (9000번)

    retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));  // 소켓과 주소 바인딩
    if (retval == SOCKET_ERROR) err_quit("bind()");  // 바인딩 실패 시 종료

    // listen()
    retval = listen(listen_sock, SOMAXCONN);  // 대기열의 크기를 최대값으로 설정
    if (retval == SOCKET_ERROR) err_quit("listen()");  // 대기 실패 시 종료

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;  // 클라이언트 소켓
    struct sockaddr_in clientaddr;  // 클라이언트 주소 구조체
    int addrlen;  // 클라이언트 주소 길이
    HANDLE hThread;  // 스레드 핸들

    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);  // 주소 구조체의 크기
        client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);  // 클라이언트 연결 수락
        if (client_sock == INVALID_SOCKET) {  // 클라이언트 연결 수락 실패 시
            err_display("accept()");
            break;  // 종료
        }

        // 접속한 클라이언트 정보 출력
        char addr[INET_ADDRSTRLEN];  // 클라이언트의 IP 주소를 저장할 배열
        inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));  // IP 주소를 문자열로 변환
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
            addr, ntohs(clientaddr.sin_port));

        // 스레드 생성 (클라이언트와의 통신을 별도의 스레드에서 처리)
        hThread = CreateThread(NULL, 0, ProcessClient,
            (LPVOID)client_sock, 0, NULL);  // 클라이언트와의 데이터 통신을 처리하는 스레드 생성
        if (hThread == NULL) {  // 스레드 생성 실패 시
            closesocket(client_sock);  // 소켓 닫기
        }
        else {  // 스레드 생성 성공 시
            CloseHandle(hThread);  // 스레드 핸들 닫기
        }
    }

    // 소켓 닫기
    closesocket(listen_sock);  // 리스닝 소켓 닫기

    // 윈속 종료
    WSACleanup();  // 윈속 종료
    return 0;  // 프로그램 종료
}
