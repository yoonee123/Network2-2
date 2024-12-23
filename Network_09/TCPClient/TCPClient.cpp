#include "..\..\Common.h"  // 공통 헤더 파일 포함 (에러 처리, 윈속 관련 함수 등)

char* SERVERIP = (char*)"127.0.0.1";  // 기본 서버 IP는 127.0.0.1 (로컬호스트)
#define SERVERPORT 9000  // 서버 포트 번호 (9000번 포트)
#define BUFSIZE    512   // 데이터 버퍼 크기 (512 바이트)

int main(int argc, char* argv[])
{
    // 서버파일은 9주차 멀티스레드 숫자야구 게임 서버 프로그램 파일
    
    int retval;  // 반환값을 저장할 변수 (각 함수 호출 후 상태 확인)

    // 명령행 인수가 있으면 서버 IP 주소를 변경
    if (argc > 1) SERVERIP = argv[1];  // 첫 번째 인수로 서버 IP 주소를 지정할 수 있음

    // 윈속 초기화
    WSADATA wsa;  // 윈속 관련 구조체
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {  // 윈속 2.2 버전 초기화
        printf("WSAStartup() 실패.\n");  // 초기화 실패 시 메시지 출력
        return 1;  // 오류 코드 1을 반환하고 종료
    }

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);  // TCP 소켓 생성 (IPv4)
    if (sock == INVALID_SOCKET) err_quit("socket()");  // 소켓 생성 실패 시 종료

    // 서버 주소 설정
    struct sockaddr_in serveraddr;  // 서버의 주소 구조체
    memset(&serveraddr, 0, sizeof(serveraddr));  // 구조체 초기화
    serveraddr.sin_family = AF_INET;  // 주소 체계: IPv4
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);  // 서버 IP 주소 설정
    serveraddr.sin_port = htons(SERVERPORT);  // 서버 포트 설정 (9000번 포트)

    // 서버와 연결 (connect)
    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));  // 서버에 연결 요청
    if (retval == SOCKET_ERROR) err_quit("connect()");  // 연결 실패 시 종료

    // 데이터 통신에 사용할 변수
    int sendData[3];  // 보낼 데이터 배열 (숫자 3개)
    int receiveData[3];  // 받을 데이터 배열 (스트라이크, 볼, 랭킹)

    // 서버와 데이터 통신
    while (1) {
        // 사용자로부터 숫자 3개 입력 받기
        printf("\n[보낼 데이터] 숫자 3개를 입력하세요 (예: 1 2 3): ");
        scanf_s("%d %d %d", &sendData[0], &sendData[1], &sendData[2]);  // 사용자 입력 받기

        // 네트워크 바이트 순서로 변환 (큰 엔디언 순서로 변환)
        for (int i = 0; i < 3; i++) {
            sendData[i] = htons(sendData[i]);  // 각 숫자를 네트워크 바이트 순서로 변환
        }

        // 데이터 보내기
        retval = send(sock, (char*)sendData, sizeof(sendData), 0);  // 서버로 데이터 전송
        if (retval == SOCKET_ERROR) {
            err_display("send()");  // 데이터 전송 실패 시 에러 출력
            break;  // 오류 발생 시 루프 종료
        }
        printf("서버로 숫자를 보냈습니다.\n");  // 숫자 전송 완료 메시지

        // 데이터 받기 (서버로부터 스트라이크, 볼, 랭킹 받기)
        retval = recv(sock, (char*)receiveData, sizeof(receiveData), 0);  // 서버로부터 데이터 수신
        if (retval <= 0) {
            printf("서버에서 연결이 종료되었습니다.\n");  // 서버 연결 종료 시 메시지 출력
            break;  // 서버 연결 종료 시 루프 종료
        }

        // 네트워크 바이트 순서를 호스트 바이트 순서로 변환 (ntohs: 네트워크에서 호스트 순서로 변환)
        int strikes = ntohs(receiveData[0]);  // 스트라이크 수
        int balls = ntohs(receiveData[1]);  // 볼 수
        int rank = ntohs(receiveData[2]);  // 랭킹

        // 결과 출력
        printf("결과: %d 스트라이크, %d 볼\n", strikes, balls);  // 스트라이크와 볼 출력

        // 3스트라이크 맞추면 게임 종료
        if (strikes == 3) {
            printf("3 스트라이크입니다. 숫자를 모두 맞췄습니다.\n");  // 3 스트라이크 맞췄을 때
            printf("당신이 숫자를 맞춘 순위는 %d 입니다.\n", rank);  // 랭킹 출력
            break;  // 게임 종료
        }
    }

    // 소켓 닫기
    closesocket(sock);  // 서버와의 연결 종료

    // 윈속 종료
    WSACleanup();  // 윈속 종료 (소켓 사용 후 클린업)

    return 0;  // 프로그램 종료
}
