#include "..\..\Common.h"  // 공통 헤더 파일을 포함하여 에러 처리 및 유틸리티 함수들을 사용할 수 있도록 함

#define SERVERPORT 9000  // 서버 포트 번호 (9000번 포트)
#define BUFSIZE    50    // 버퍼 크기 (최대 50바이트)

int main(int argc, char *argv[])
{
    int retval;  // 반환 값 저장 변수 (함수 호출 결과 체크)
    int len;     // 데이터 길이 저장 변수

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
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  // 모든 네트워크 인터페이스에서 수신
    serveraddr.sin_port = htons(SERVERPORT);  // 포트 번호 설정 (9000번)

    retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));  // 소켓에 주소 할당
    if (retval == SOCKET_ERROR) err_quit("bind()");  // 바인드 실패 시 종료

    // listen()
    retval = listen(listen_sock, SOMAXCONN);  // 연결 요청을 받을 준비
    if (retval == SOCKET_ERROR) err_quit("listen()");  // 리슨 실패 시 종료

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;  // 클라이언트 소켓
    struct sockaddr_in clientaddr;  // 클라이언트 주소 구조체
    int addrlen;  // 주소 길이
    char buf[BUFSIZE + 1];  // 데이터 버퍼

    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);  // 클라이언트 연결 수락
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");  // 수락 실패 시 에러 출력
            break;
        }

        // 접속한 클라이언트 정보 출력
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));  // 클라이언트 IP 주소 가져오기
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
            addr, ntohs(clientaddr.sin_port));  // 접속한 클라이언트의 IP와 포트 출력

        // 클라이언트와 데이터 통신
        while (1) {
            // 데이터 받기
            retval = recv(client_sock, buf, BUFSIZE, MSG_WAITALL);  // 클라이언트로부터 데이터 받기
            if (retval == SOCKET_ERROR) {
                err_display("recv()");  // 수신 실패 시 에러 출력
                break;
            }
            else if (retval == 0)
                break;  // 클라이언트가 연결을 끊으면 종료

            // 받은 데이터 출력
            buf[retval] = '\0';  // 받은 데이터를 null로 종료
            printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);  // 받은 메시지 출력

            // 클라이언트로 메시지 쏘기
            for (int i = 0; i < 4; i++) {
                printf("\n메시지를 입력하세요.\n");
                if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
                    break;  // 입력 받기 실패 시 종료

                len = (int)strlen(buf);  // 입력 받은 메시지 길이 계산
                if (buf[len - 1] == '\n')  // 개행 문자 제거
                    buf[len - 1] = '\0';
                if (strlen(buf) == 0)  // 빈 메시지 입력 시 종료
                    break;

                // 데이터 보내기
                retval = send(client_sock, buf, BUFSIZE, 0);  // 클라이언트로 메시지 전송
                if (retval == SOCKET_ERROR) {
                    err_display("send()");  // 송신 실패 시 에러 출력
                    break;
                }
                printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);  // 보낸 데이터 크기 출력

                // 데이터 받기
                retval = recv(client_sock, buf, BUFSIZE, 0);  // 클라이언트로부터 응답 받기
                if (retval == SOCKET_ERROR) {
                    err_display("recv()");  // 수신 실패 시 에러 출력
                    break;
                }
                else if (retval == 0)
                    break;  // 클라이언트가 연결을 끊으면 종료

                buf[retval] = '\0';  // 받은 데이터 null로 종료
                printf("서버로부터 메시지를 받았습니다.\n");
                printf("[받은 데이터] : %s\n", buf);  // 받은 메시지 출력
            }
        }

        // 소켓 닫기
        closesocket(client_sock);  // 클라이언트와의 연결 종료
        printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
            addr, ntohs(clientaddr.sin_port));  // 클라이언트 종료 메시지 출력
    }

    // 소켓 닫기
    closesocket(listen_sock);  // 서버 소켓 종료

    // 윈속 종료
    WSACleanup();  // 윈속 종료
    return 0;  // 프로그램 종료
}
