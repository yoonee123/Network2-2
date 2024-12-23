#include "..\..\Common.h"  // 공통 헤더 파일을 포함하여 에러 처리 및 유틸리티 함수들을 사용할 수 있도록 함

char *SERVERIP = (char *)"127.0.0.1";  // 기본 서버 IP 주소 (127.0.0.1: 로컬호스트)
#define SERVERPORT 9000  // 서버 포트 번호 (9000번 포트)
#define BUFSIZE    50    // 버퍼 크기 (최대 50바이트)

int main(int argc, char *argv[])
{
    int retval;  // 반환 값 저장 변수 (함수 호출 결과 체크)

    // 명령행 인수가 있으면 IP 주소로 사용
    if (argc > 1) SERVERIP = argv[1];  // 명령행 인수로 IP 주소를 지정할 수 있음

    // 윈속 초기화
    WSADATA wsa;  // 윈속 관련 데이터 구조체
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)  // 윈속 초기화 (버전 2.2)
        return 1;  // 초기화 실패 시 1을 반환하고 종료

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);  // TCP 소켓 생성
    if (sock == INVALID_SOCKET) err_quit("socket()");  // 소켓 생성 실패 시 종료

    // connect()
    struct sockaddr_in serveraddr;  // 서버 주소 구조체
    memset(&serveraddr, 0, sizeof(serveraddr));  // 구조체 초기화
    serveraddr.sin_family = AF_INET;  // 주소 체계: IPv4
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);  // 서버 IP 주소 설정
    serveraddr.sin_port = htons(SERVERPORT);  // 포트 번호 설정 (9000번)

    retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));  // 서버에 연결
    if (retval == SOCKET_ERROR) err_quit("connect()");  // 연결 실패 시 종료

    // 데이터 통신에 사용할 변수
    char buf[BUFSIZE];  // 데이터 송수신 버퍼
    const char *testdata[] = {  // 서버에 전송할 테스트 데이터 배열
        "안녕하세요",
        "반가워요",
        "오늘따라 할 이야기가 많을 것 같네요",
        "저도 그렇네요",
    };
    int len;  // 전송할 데이터의 길이

    // 서버와 데이터 통신
    for (int i = 0; i < 4; i++) {
        // 데이터 입력(시뮬레이션)
        len = (int)strlen(testdata[i]);  // 테스트 데이터의 길이
        strncpy(buf, testdata[i], len);  // 테스트 데이터를 버퍼에 복사

        int big = htonl(len);  // 데이터 길이를 네트워크 바이트 순서로 변환

        // 데이터 보내기(고정 길이)
        retval = send(sock, (char *)&big, sizeof(int), 0);  // 먼저 데이터 길이를 보내기
        if (retval == SOCKET_ERROR) {
            err_display("send()");  // 송신 실패 시 에러 메시지 출력
            break;
        }

        // 데이터 보내기(가변 길이)
        retval = send(sock, buf, len, 0);  // 실제 데이터를 보내기
        if (retval == SOCKET_ERROR) {
            err_display("send()");  // 송신 실패 시 에러 메시지 출력
            break;
        }
        printf("[TCP 클라이언트] %d+%d바이트를 보냈습니다.\n", (int)sizeof(int), retval);  // 보낸 데이터 크기 출력
    }

    // 소켓 닫기
    closesocket(sock);  // 소켓을 닫아 연결 종료

    // 윈속 종료
    WSACleanup();  // 윈속 종료
    return 0;  // 프로그램 종료
}
