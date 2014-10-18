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
	if (!IsWindow(gSaiWnds.main))
		GetSaiWindowAll(&gSaiWnds);
	return IsWindow(gSaiWnds.main) ? gSaiWnds.main : NULL;
}

HOOKDLL_API DWORD _stdcall SetSaiHook(HINSTANCE hInst) {
	DWORD dwThread = 0;
	if (GetSaiWindow() != NULL) {
		dwThread = GetWindowThreadProcessId(gSaiWnds.main, NULL);
		if (CheckSaiWindowList(&gSaiWnds) >= 0)
			GetSaiWindowAll(&gSaiWnds);
	}
	if (gStatus.targetThread != dwThread) {
		UnsetSaiHook();
		if (dwThread != 0) {
			gMsgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, hInst, dwThread);
			gProcHook = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, hInst, dwThread);
		}
		gStatus.targetThread = dwThread;
		gStatus.notifyThread = GetCurrentThreadId();
//		AttachThreadInput(gStatus.notifyThread, gStatus.targetThread, TRUE);
	}
	if (dwThread == 0)
		return 1444; // Invalid Thread Identifier
	else
		return gMsgHook != NULL && gProcHook != NULL ? 0 : GetLastError();
}

HOOKDLL_API void _stdcall UnsetSaiHook() {
	if (gStatus.targetThread != 0)
		EnumThreadWindows(gStatus.targetThread, SendQuitMsgProc, NULL);
	if (gMsgHook != NULL)
		UnhookWindowsHookEx(gMsgHook);
	if (gProcHook != NULL)
		UnhookWindowsHookEx(gProcHook);
//	AttachThreadInput(gStatus.notifyThread, gStatus.targetThread, TRUE);
	gMsgHook = gProcHook = NULL;
	gStatus.targetThread = 0;
}

HOOKDLL_API int _stdcall SaiStatus(TCHAR *szKey, TCHAR *szVal, TCHAR* szRet, int size) {
	// setter
	if (szVal) {
		if (!_tcscmp(szKey, TEXT("lock-touch")))
			gSettings.lockTouch = _tcslen(szVal) ? 1 : 0;
		else if (!_tcscmp(szKey, TEXT("keep-mousegesture")))
			gSettings.keepMouseGesture = szVal && szVal[0];
	}
	// getter
	if (szRet) {
		if (!_tcscmp(szKey, TEXT("lock-touch")))
			StringCbCopy(szRet, size, gSettings.lockTouch ? TEXT("1") : TEXT(""));
		else if (!_tcscmp(szKey, TEXT("gesture-vector")))
			StringCbCopy(szRet, size, gStatus.mgVectorStr);
		else if (!_tcscmp(szKey, TEXT("zoom")))
			GetWindowText(gSaiWnds.zoom, szRet, size);
		else if (!_tcscmp(szKey, TEXT("rotate")))
			GetWindowText(gSaiWnds.rotate, szRet, size);
		else if (!_tcscmp(szKey, TEXT("pen"))) {
			SendMessage(gSaiWnds.tools, WM_USER_GET_PEN, 0, 0);
			_stprintf_s(szRet, size, TEXT("%dpx %s"), gStatus.penSize, gStatus.penName);
		}
	}
	return 0;
}

HOOKDLL_API void _stdcall SimulateKeyEvent(int vk, bool down) {
	SimulateKey(vk, down ? 0 : KEYEVENTF_KEYUP);
}

HOOKDLL_API void _stdcall SimulateMouseEvent(int x, int y, int leftDown, int rightDown) {
	SimulateMouse(x, y, 0,
		(leftDown ? (leftDown > 0 ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP) : 0) |
		(rightDown? (rightDown> 0 ? MOUSEEVENTF_RIGHTDOWN: MOUSEEVENTF_RIGHTUP): 0));
}

HOOKDLL_API void _stdcall SimulateDragWithKey(int vk, bool ctrl, bool shift, bool alt) {
	SHORTCUT_KEY *pk = &gSettings.dragKey;
	if (pk->enabled)
		return;
	pk->enabled = TRUE;
	pk->vk = (WORD)vk;
	pk->ctrl = ctrl;
	pk->shift = shift;
	pk->alt = alt;
	SimulateShortcut(pk, TRUE);
	// an additional MOUSEEVENTF_RIGHTUP is required for mouse gesture
	if (gStatus.isRightDown)
		SimulateMouse(0, 0, 0, MOUSEEVENTF_RIGHTUP);
	SimulateMouse(0, 0, 0, MOUSEEVENTF_LEFTDOWN);
}

HOOKDLL_API void _stdcall RegisterEventNotify(int msg, TCHAR *evt, TCHAR *steps) {
	EVENT_TRIGGER *pe = NULL;
	double val = 0;
	if (!_tcscmp(evt, TEXT("ms-x"))) {
		pe = &gSettings.evtOffsetX;
		val = gStatus.penHoverPos.x;
	}
	else if (!_tcscmp(evt, TEXT("ms-y"))) {
		pe = &gSettings.evtOffsetY;
		val = gStatus.penHoverPos.y;
	}
	else if (!_tcscmp(evt, TEXT("th-z"))) {
		pe = &gSettings.evtZoom;
		val = gStatus.tgScale;
	}
	else if (!_tcscmp(evt, TEXT("th-r"))) {
		pe = &gSettings.evtRotate;
		val = gStatus.tgRotate;
	}
	if (pe) {
		pe->msg = msg;
		pe->size = pe->index = 0;
		for (int i = 0, j = 0, n = _tcslen(steps); i <= n && pe->size < MAX_SETTING_STEPS; i ++) {
			if (steps[i] == ',' || steps[i] == '\0') {
				steps[i] = '\0';
				pe->list[pe->size ++] = _ttof(steps + j) + val;
				j = ++i;
			}
		}
		while(ListIndex(pe, val));
	}
}
