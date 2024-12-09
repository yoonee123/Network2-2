#include "..\..\Common.h"
#include "resource.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512

// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 에디트 컨트롤 출력 함수
void DisplayText(const char* fmt, ...);
// 소켓 함수 오류 출력
void DisplayError(const char* msg);
// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID arg);

SOCKET sock;                 // 소켓
char buf[BUFSIZE + 1];       // 데이터 송수신 버퍼
HANDLE hReadEvent, hWriteEvent; // 이벤트
HWND hSendButton;            // 보내기 버튼
HWND hEdit1, hEdit2;         // 에디트 컨트롤
struct sockaddr_in serveraddr; // 서버 주소 구조체

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // 이벤트 생성
    hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // 소켓 통신 스레드 생성
    CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

    // 대화상자 생성
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

    // 이벤트 제거
    CloseHandle(hReadEvent);
    CloseHandle(hWriteEvent);

    // 윈속 종료
    WSACleanup();
    return 0;
}

// 대화상자 프로시저
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
            EnableWindow(hSendButton, FALSE); // 보내기 버튼 비활성화
            WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 대기
            GetDlgItemTextA(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
            SetEvent(hWriteEvent); // 쓰기 완료 알림
            SetFocus(hEdit1); // 키보드 포커스 전환
            SendMessage(hEdit1, EM_SETSEL, 0, -1); // 텍스트 전체 선택
            return TRUE;
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL); // 대화상자 닫기
            closesocket(sock); // 소켓 닫기
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// 에디트 컨트롤 출력 함수
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

// 소켓 함수 오류 출력
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

// UDP 클라이언트 시작 부분
DWORD WINAPI ClientMain(LPVOID arg)
{
    int retval;

    // 소켓 생성
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // 서버 주소 구조체 초기화
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);

    // 서버와 데이터 통신
    struct sockaddr_in peeraddr;
    int addrlen;
    while (1) {
        WaitForSingleObject(hWriteEvent, INFINITE); // 쓰기 완료 대기

        // 문자열 길이가 0이면 보내지 않음
        if (strlen(buf) == 0) {
            EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
            SetEvent(hReadEvent); // 읽기 완료 알림
            continue;
        }

        // 데이터 보내기
        retval = sendto(sock, buf, (int)strlen(buf), 0,
            (struct sockaddr*)&serveraddr, sizeof(serveraddr));
        if (retval == SOCKET_ERROR) {
            DisplayError("sendto()");
            break;
        }
        DisplayText("[UDP 클라이언트] %d바이트를 보냈습니다.\r\n", retval);

        // 데이터 받기
        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0,
            (struct sockaddr*)&peeraddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            DisplayError("recvfrom()");
            break;
        }

        // 송신자의 주소 체크
        if (memcmp(&peeraddr, &serveraddr, sizeof(peeraddr))) {
            DisplayText("[오류] 잘못된 데이터입니다!\r\n");
            break;
        }

        // 받은 데이터 출력
        buf[retval] = '\0';
        DisplayText("[UDP 클라이언트] %d바이트를 받았습니다.\r\n", retval);
        DisplayText("[받은 데이터] %s\r\n", buf);

        EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
        SetEvent(hReadEvent); // 읽기 완료 알림
    }

    return 0;
}
