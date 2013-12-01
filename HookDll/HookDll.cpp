// HookDll.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "Shared.h"
#include "Callbacks.h"
#include "HookDll.h"

/*
// ���ǵ���������һ��ʾ��
HOOKDLL_API int nHookDll=0;

// ���ǵ���������һ��ʾ����
HOOKDLL_API int fnHookDll(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� HookDll.h
CHookDll::CHookDll()
{
	return;
}
*/

#define SAI_WINDOW_CLASS TEXT("sfl_window_class")
#define SAI_MENUBAR_CLASS TEXT("sfl_menubar_class")

static HHOOK gMsgHook = NULL;
static HHOOK gProcHook = NULL;

HOOKDLL_API HWND _stdcall GetSaiWindow() {
	HWND hWnd = NULL;
	while (hWnd = FindWindowEx(NULL, hWnd, SAI_WINDOW_CLASS, NULL)) {
		if (GetWindowTextLength(hWnd) > 0 &&
			FindWindowEx(hWnd, NULL, SAI_MENUBAR_CLASS, NULL) != NULL) {
			break;
		}
	}
	return hWnd;

}

HOOKDLL_API HWND _stdcall SetNotifyWindow(HWND hWnd) {
	HWND hLast = gSettings.nofityWnd;
	gSettings.nofityWnd = hWnd;
	return hLast;
}

HOOKDLL_API DWORD _stdcall SetSaiHook(HINSTANCE hInst) {
	HWND hWnd = GetSaiWindow();
	DWORD dwThread = hWnd == NULL ? 0 : GetWindowThreadProcessId(hWnd, NULL);
	if (gStatus.threadId != dwThread)
		UnsetSaiHook();
	if (gStatus.threadId != dwThread) {
		if (dwThread != 0) {
			gMsgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, hInst, dwThread);
			gProcHook = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, hInst, dwThread);
		}
//		LogText(TEXT("thread hook: hooked to thread %d (%d, %d)\r\n"), dwThread, gMsgHook, gProcHook);
//		PostMessage(gSettings.nofityWnd, WM_USER+WM_APP, dwThread, 1);
		gStatus.threadId = dwThread;
	}
	if (dwThread == 0)
		return 1444; //Invalid Thread Identifier
	else
		return gMsgHook != NULL && gProcHook != NULL ? 0 : GetLastError();
}

HOOKDLL_API void _stdcall UnsetSaiHook() {
	if (gMsgHook != NULL)
		UnhookWindowsHookEx(gMsgHook);
	if (gProcHook != NULL)
		UnhookWindowsHookEx(gProcHook);
	if (gStatus.threadId != 0) {
		EnumThreadWindows(gStatus.threadId, SendQuitMsgProc, NULL);
//		LogText(TEXT("thread hook: unhooked\r\n"));
//		PostMessage(gSettings.nofityWnd, WM_USER+WM_APP, 0, 2);
	}
	gStatus.threadId = 0;
	gMsgHook = gProcHook = NULL;
}

HOOKDLL_API int _stdcall LockTouch(int lock) {
	if (lock > 0)
		gSettings.lockTouch = 1;
	else if (lock == 0)
		gSettings.lockTouch = 0;
	return gSettings.lockTouch;
}

HOOKDLL_API int _stdcall SetPanningVk(int vk) {
	if (vk >= 0)
		gSettings.panVkCode = vk;
	return gSettings.panVkCode;
}

HOOKDLL_API int _stdcall GetVectorStr(TCHAR* szBuf, int size) {
	return StringCbCopy(szBuf, size, gStatus.vectorStr);
}

HOOKDLL_API void _stdcall SimulateKeyEvent(int vk, bool down) {
	SimulateKey(vk, down ? 0 : KEYEVENTF_KEYUP);
}

HOOKDLL_API void _stdcall SimulateMouseEvent(int x, int y, bool down) {
	SimulateMouse(x, y, 0, down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP);
}