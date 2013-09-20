#define EVENTF_FROMTOUCH 0xFF515700

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK SendQuitMsgProc(HWND hWnd, LPARAM lParam);