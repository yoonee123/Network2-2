#include "..\..\Common.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

int main(int argc, char* argv[])
{
    // 서버파일은 9주차 멀티스레드 숫자야구 게임 서버 프로그램 파일

    int retval;

    // 명령행 인수가 있으면 IP 주소로 사용
    if (argc > 1) SERVERIP = argv[1];

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup() 실패.\n");
        return 1;
    }

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // connect()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    // 데이터 통신에 사용할 변수
    int sendData[3];
    int receiveData[3];

    // 서버와 데이터 통신
    while (1) {
        // 숫자 3개 입력
        printf("\n[보낼 데이터] 숫자 3개를 입력하세요 (예: 1 2 3): ");
        scanf_s("%d %d %d", &sendData[0], &sendData[1], &sendData[2]);

        // 네트워크 바이트 순서로 변환
        for (int i = 0; i < 3; i++) {
            sendData[i] = htons(sendData[i]);
        }

        // 데이터 보내기
        retval = send(sock, (char*)sendData, sizeof(sendData), 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
        printf("서버로 숫자를 보냈습니다.\n");

        // 데이터 받기 (스트라이크, 볼, 랭킹)
        retval = recv(sock, (char*)receiveData, sizeof(receiveData), 0);
        if (retval <= 0) {
            printf("서버에서 연결이 종료되었습니다.\n");
            break;
        }

        // 네트워크 바이트 순서를 호스트 바이트 순서로 변환
        int strikes = ntohs(receiveData[0]);
        int balls = ntohs(receiveData[1]);
        int rank = ntohs(receiveData[2]);

        // 결과 출력
        printf("결과: %d 스트라이크, %d 볼\n", strikes, balls);

        // 3스트라이크 확인
        if (strikes == 3) {
            printf("3 스트라이크입니다. 숫자를 모두 맞췄습니다.\n");
            printf("당신이 숫자를 맞춘 순위는 %d 입니다.\n", rank);
            break;
        }
    }

    // 소켓 닫기
    closesocket(sock);

    // 윈속 종료
    WSACleanup();
    return 0;
}
