#include "stdafx.h"
#include "Shared.h"

#pragma data_seg("Share")

SETTINGS gSettings = { 0 };
STATUS gStatus = { 0 };
SAI_WINDOWS gSaiWnds = { 0 };

#pragma data_seg()
#pragma comment(linker, "/Section:Share,rws")

int ListIndex(EVENT_TRIGGER *pl, double val) {
	int delta = 0;
	if (pl->index >= 1 && val < pl->list[pl->index])
		delta = -1;
	if (pl->index + 1 < pl->size - 1 && val > pl->list[pl->index + 1])
		delta = 1;
	pl->index += delta;
	return delta;
}

void SimulateShortcut(SHORTCUT_KEY *pk, BOOL down) {
	if (down) {
		if (pk->ctrl)
			SimulateKey(VK_CONTROL, 0);
		if (pk->shift)
			SimulateKey(VK_SHIFT, 0);
		if (pk->alt)
			SimulateKey(VK_MENU, 0);
		if (pk->vk)
			SimulateKey(pk->vk, 0);
	}
	else {
		if (pk->vk)
			SimulateKey(pk->vk, KEYEVENTF_KEYUP);
		if (pk->ctrl)
			SimulateKey(VK_CONTROL, KEYEVENTF_KEYUP);
		if (pk->shift)
			SimulateKey(VK_SHIFT, KEYEVENTF_KEYUP);
		if (pk->alt)
			SimulateKey(VK_MENU, KEYEVENTF_KEYUP);
	}
}

void SimulateKey(WORD vk, DWORD flags) {
	KEYBDINPUT ki = {vk, MapVirtualKey(vk, MAPVK_VK_TO_VSC), flags, 0, LLMHF_INJECTED};
	INPUT ip; ip.type = INPUT_KEYBOARD; ip.ki = ki;
	SendInput(1, &ip, sizeof(INPUT));
}

void SimulateMouse(LONG dx, LONG dy, DWORD data, DWORD flags) {
	MOUSEINPUT mi = {dx, dy, data, flags, 0, LLMHF_INJECTED};
	INPUT ip; ip.type = INPUT_MOUSE; ip.mi = mi;
	SendInput(1, &ip, sizeof(INPUT));
}

void GetChildWindowList(HWND hParent, WINDOW_LIST *pls) {
	pls->size = 0;
	HWND hWnd = NULL;
	while (hWnd = FindWindowEx(hParent, hWnd, NULL, NULL)) {
		pls->list[pls->size ++] = hWnd;
	}
}

int CheckSaiWindowList(SAI_WINDOWS *psw) {
	HWND *pls = (HWND *)psw;
	for (int i = 0; i < sizeof(SAI_WINDOWS)/sizeof(HWND); i ++) {
		if (!IsWindow(pls[i])) {
			PostNotify(WM_USER_DEBUG, i, 0);
			return i;
		}
	}
	return -1;
}

void CheckSaiWindow(HWND hWnd, SAI_WINDOWS *psw) {
	if (GetDlgCtrlID(hWnd) == 0x800) {
		psw->canvas = hWnd;
		return;
	}
	WINDOW_LIST wls;
	GetChildWindowList(hWnd, &wls);
	if (wls.size) {
		int dlgId = GetDlgCtrlID(wls.list[0]);
		if (dlgId == 0x0000) {
			if (wls.size >=2 && GetDlgCtrlID(wls.list[1]) == 0x0201) {
				psw->nav = hWnd;
			}
		}
		else if (dlgId == 0x0301) {
			psw->layers = hWnd;
		}
		else if (dlgId == 0x0401) {
			psw->color = hWnd;
		}
		else if (dlgId == 0x0501) {
			psw->top = hWnd;
			psw->zoom = wls.list[5];
			psw->rotate = wls.list[9];
		}
		else if (dlgId == 0x0601) {
			psw->tools = hWnd;
		}
	}
}

void GetSaiWindowAll(SAI_WINDOWS *psw) {
	HWND hWnd = NULL;
	while (hWnd = FindWindowEx(NULL, hWnd, SAI_WINDOW_CLASS, NULL)) {
		HWND hMenu = FindWindowEx(hWnd, NULL, SAI_MENUBAR_CLASS, NULL);
		if (hMenu != NULL) {
			psw->main = hWnd;
			psw->menu = hMenu;
		}
		else {
			CheckSaiWindow(hWnd, psw);
		}
	}
	HWND hChild = NULL;
	while (hChild = FindWindowEx(psw->main, hChild, SAI_WINDOW_CLASS, NULL)) {
		CheckSaiWindow(hChild, psw);
	}
}

BOOL IsSaiCanvasWindow(HWND hWnd) {
	return hWnd == gSaiWnds.canvas || GetParent(hWnd) == gSaiWnds.canvas;
}