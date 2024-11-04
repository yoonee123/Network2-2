#include "..\..\Common.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

int main(int argc, char* argv[])
{
    // ���������� 9���� ��Ƽ������ ���ھ߱� ���� ���� ���α׷� ����

    int retval;

    // ����� �μ��� ������ IP �ּҷ� ���
    if (argc > 1) SERVERIP = argv[1];

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup() ����.\n");
        return 1;
    }

    // ���� ����
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

    // ������ ��ſ� ����� ����
    int sendData[3];
    int receiveData[3];

    // ������ ������ ���
    while (1) {
        // ���� 3�� �Է�
        printf("\n[���� ������] ���� 3���� �Է��ϼ��� (��: 1 2 3): ");
        scanf_s("%d %d %d", &sendData[0], &sendData[1], &sendData[2]);

        // ��Ʈ��ũ ����Ʈ ������ ��ȯ
        for (int i = 0; i < 3; i++) {
            sendData[i] = htons(sendData[i]);
        }

        // ������ ������
        retval = send(sock, (char*)sendData, sizeof(sendData), 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
        printf("������ ���ڸ� ���½��ϴ�.\n");

        // ������ �ޱ� (��Ʈ����ũ, ��, ��ŷ)
        retval = recv(sock, (char*)receiveData, sizeof(receiveData), 0);
        if (retval <= 0) {
            printf("�������� ������ ����Ǿ����ϴ�.\n");
            break;
        }

        // ��Ʈ��ũ ����Ʈ ������ ȣ��Ʈ ����Ʈ ������ ��ȯ
        int strikes = ntohs(receiveData[0]);
        int balls = ntohs(receiveData[1]);
        int rank = ntohs(receiveData[2]);

        // ��� ���
        printf("���: %d ��Ʈ����ũ, %d ��\n", strikes, balls);

        // 3��Ʈ����ũ Ȯ��
        if (strikes == 3) {
            printf("3 ��Ʈ����ũ�Դϴ�. ���ڸ� ��� ������ϴ�.\n");
            printf("����� ���ڸ� ���� ������ %d �Դϴ�.\n", rank);
            break;
        }
    }

    // ���� �ݱ�
    closesocket(sock);

    // ���� ����
    WSACleanup();
    return 0;
}
