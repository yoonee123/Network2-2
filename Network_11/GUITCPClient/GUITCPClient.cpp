#include "..\..\Common.h"
#include "resource.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512

// ��ȭ���� ���ν���
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// ����Ʈ ��Ʈ�� ��� �Լ�
void DisplayText(const char* fmt, ...);
// ���� �Լ� ���� ���
void DisplayError(const char* msg);
// ���� ��� ������ �Լ�
DWORD WINAPI ClientMain(LPVOID arg);

SOCKET sock;                 // ����
char buf[BUFSIZE + 1];       // ������ �ۼ��� ����
HANDLE hReadEvent, hWriteEvent; // �̺�Ʈ
HWND hSendButton;            // ������ ��ư
HWND hEdit1, hEdit2;         // ����Ʈ ��Ʈ��
struct sockaddr_in serveraddr; // ���� �ּ� ����ü

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // �̺�Ʈ ����
    hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // ���� ��� ������ ����
    CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

    // ��ȭ���� ����
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

    // �̺�Ʈ ����
    CloseHandle(hReadEvent);
    CloseHandle(hWriteEvent);

    // ���� ����
    WSACleanup();
    return 0;
}

// ��ȭ���� ���ν���
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
        hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
        hSendButton = GetDlgItem(hDlg, IDOK);
        SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EnableWindow(hSendButton, FALSE); // ������ ��ư ��Ȱ��ȭ
            WaitForSingleObject(hReadEvent, INFINITE); // �б� �Ϸ� ���
            GetDlgItemTextA(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
            SetEvent(hWriteEvent); // ���� �Ϸ� �˸�
            SetFocus(hEdit1); // Ű���� ��Ŀ�� ��ȯ
            SendMessage(hEdit1, EM_SETSEL, 0, -1); // �ؽ�Ʈ ��ü ����
            return TRUE;
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL); // ��ȭ���� �ݱ�
            closesocket(sock); // ���� �ݱ�
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// ����Ʈ ��Ʈ�� ��� �Լ�
void DisplayText(const char* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    char cbuf[BUFSIZE * 2];
    vsprintf(cbuf, fmt, arg);
    va_end(arg);

    int nLength = GetWindowTextLength(hEdit2);
    SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
    SendMessageA(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
}

// ���� �Լ� ���� ���
void DisplayError(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&lpMsgBuf, 0, NULL);
    DisplayText("[%s] %s\r\n", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// UDP Ŭ���̾�Ʈ ���� �κ�
DWORD WINAPI ClientMain(LPVOID arg)
{
    int retval;

    // ���� ����
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // ���� �ּ� ����ü �ʱ�ȭ
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);

    // ������ ������ ���
    struct sockaddr_in peeraddr;
    int addrlen;
    while (1) {
        WaitForSingleObject(hWriteEvent, INFINITE); // ���� �Ϸ� ���

        // ���ڿ� ���̰� 0�̸� ������ ����
        if (strlen(buf) == 0) {
            EnableWindow(hSendButton, TRUE); // ������ ��ư Ȱ��ȭ
            SetEvent(hReadEvent); // �б� �Ϸ� �˸�
            continue;
        }

        // ������ ������
        retval = sendto(sock, buf, (int)strlen(buf), 0,
            (struct sockaddr*)&serveraddr, sizeof(serveraddr));
        if (retval == SOCKET_ERROR) {
            DisplayError("sendto()");
            break;
        }
        DisplayText("[UDP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\r\n", retval);

        // ������ �ޱ�
        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0,
            (struct sockaddr*)&peeraddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            DisplayError("recvfrom()");
            break;
        }

        // �۽����� �ּ� üũ
        if (memcmp(&peeraddr, &serveraddr, sizeof(peeraddr))) {
            DisplayText("[����] �߸��� �������Դϴ�!\r\n");
            break;
        }

        // ���� ������ ���
        buf[retval] = '\0';
        DisplayText("[UDP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\r\n", retval);
        DisplayText("[���� ������] %s\r\n", buf);

        EnableWindow(hSendButton, TRUE); // ������ ��ư Ȱ��ȭ
        SetEvent(hReadEvent); // �б� �Ϸ� �˸�
    }

    return 0;
}
