#include "..\..\Common.h"  // 공통 헤더 파일 포함 (에러 처리 함수나 공통 설정이 있을 수 있음)

char *SERVERIP = (char *)"127.0.0.1";  // 기본 서버 IP 주소 (로컬호스트로 설정)
#define SERVERPORT 9000  // 서버 포트 번호 (9000번 포트)
#define BUFSIZE    50    // 버퍼 크기 (최대 50바이트)

int main(int argc, char *argv[])
{
    int retval;  // 반환 값 저장 변수 (함수 호출 결과 체크)
    int len;     // 데이터의 길이

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
    char buf[BUFSIZE + 1];  // 데이터 송수신 버퍼
    const char *testdata[] = {  // 테스트용 데이터 배열 (사용되지 않음, 대신 사용자 입력을 받음)
        "안녕하세요",
        "반가워요",
        "오늘따라 할 이야기가 많을 것 같네요",
        "저도 그렇네요",
    };

    // 서버와 데이터 통신
    for (int i = 0; i < 4; i++) {
        // 데이터 입력(시뮬레이션)
        // 위의 testdata[] 배열을 사용한 예시 대신, 사용자로부터 메시지를 입력 받음
        printf("\n메시지를 입력하세요.\n");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)  // 사용자 입력 받기
            break;
        len = (int)strlen(buf);  // 입력받은 문자열의 길이 계산
        if (buf[len - 1] == '\n') 
            buf[len - 1] = '\0';  // 입력받은 문자열에서 줄바꿈 문자 제거
        if (strlen(buf) == 0)  // 빈 문자열일 경우 종료
            break;

        // 데이터 보내기
        retval = send(sock, buf, BUFSIZE, 0);  // 서버로 메시지 전송
        if (retval == SOCKET_ERROR) {
            err_display("send()");  // 송신 실패 시 에러 메시지 출력
            break;
        }
        printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);

        // 데이터 받기
        retval = recv(sock, buf, BUFSIZE, 0);  // 서버로부터 메시지 받기
        if (retval == SOCKET_ERROR) {
            err_display("recv()");  // 수신 실패 시 에러 메시지 출력
            break;
        }
        else if (retval == 0)  // 서버가 연결을 종료한 경우
            break;
            
        buf[retval] = '\0';  // 받은 데이터의 끝에 널 문자 추가
        printf("클라이언트로부터 메시지를 받았습니다.\n");
        printf("[받은 데이터] : %s\n", buf);  // 받은 데이터 출력
    }

    // 소켓 닫기
    closesocket(sock);  // 소켓 닫기

    // 윈속 종료
    WSACleanup();  // 윈속 종료
    return 0;  // 프로그램 종료
}
