#include "..\..\Common.h"

char *SERVERIP = (char *)"223.194.167.227";
//#define SERVERPORT 9000
#define BUFSIZE    512

int main(int argc, char *argv[])
{
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];
	
	unsigned short SERVERPORT = 9001;
	if (argc > 2)
		SERVERPORT = atoi(argv[2]);
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE + 1];
	int len;

	int num[2];  //보낼 숫자
	int sum;  // 서버로부터 계산 결과를 받는 변수

	// 서버와 데이터 통신
	while (1) {
		// 데이터 입력
		printf("\n[보낼 데이터 : 정수 두 개 입력] ");
		scanf("%d%d", &num[0], &num[1]);
		printf("\n[보낼 데이터] ");
		//if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
		//	break;

		// '\n' 문자 제거
		//len = (int)strlen(buf);
		//if (buf[len - 1] == '\n')
		//	buf[len - 1] = '\0';
		//if (strlen(buf) == 0)
		//	break;

		// 데이터 보내기
		retval = send(sock, ( char *)num, sizeof(num), 0);
		//retval = send(sock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);

		// 데이터 받기
		retval = recv(sock, (char *) &sum, sizeof(sum), MSG_WAITALL);
		//retval = recv(sock, buf, retval, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
		printf("[받은 데이터 : 두 정수의 합은 ] %d\n", sum);
	}

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
