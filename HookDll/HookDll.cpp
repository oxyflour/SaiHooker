// HookDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Shared.h"
#include "Callbacks.h"
#include "HookDll.h"

/*
// 这是导出变量的一个示例
HOOKDLL_API int nHookDll=0;

// 这是导出函数的一个示例。
HOOKDLL_API int fnHookDll(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 HookDll.h
CHookDll::CHookDll()
{
	return;
}
*/

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
	DWORD dwProcess = 0, dwThread = 0;
	if (hWnd != NULL)
		dwThread = GetWindowThreadProcessId(hWnd, &dwProcess);
	if (gStatus.threadId != dwThread)
		UnsetSaiHook();
	if (gStatus.threadId != dwThread) {
		if (dwThread != 0) {
			gMsgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, hInst, dwThread);
			gProcHook = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, hInst, dwThread);
		}
		gStatus.threadId = dwThread;
	}
	if (dwThread == 0)
		return 1444; // Invalid Thread Identifier
	else
		return gMsgHook != NULL && gProcHook != NULL ? 0 : GetLastError();
}

HOOKDLL_API void _stdcall UnsetSaiHook() {
	if (gMsgHook != NULL)
		UnhookWindowsHookEx(gMsgHook);
	if (gProcHook != NULL)
		UnhookWindowsHookEx(gProcHook);
	if (gStatus.threadId != 0)
		EnumThreadWindows(gStatus.threadId, SendQuitMsgProc, NULL);
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

HOOKDLL_API int _stdcall AddButton(TCHAR* szText, int left, int right, int top, int bottom, int index) {
	for (int i = 0; i < MAX_BUTTON_COUNT && index < 0; i ++) {
		if (gSettings.ButtonRects[i].str[0] == 0) {
			index = i;
			break;
		}
	}
	if (index >= 0 && index < MAX_BUTTON_COUNT) {
		BUTTON_RECT* prt = gSettings.ButtonRects + index;
		if (szText)
			StringCbCopy(prt->str, MAX_BUTTON_TEXT, szText);
		else
			prt->str[0] = 0;
		prt->left = left;
		prt->right = right;
		prt->top = top;
		prt->bottom = bottom;
	}
	return index;
}