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
	WINDOW_LIST wls;
	GetChildWindowList(hWnd, &wls);
	if (wls.size >= 1) {
		BOOL find = TRUE;
		LONG style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		for (DWORD i = 0; i < wls.size; i ++) {
			if ((GetWindowLongPtr(wls.list[i], GWL_STYLE) & style) != style) {
				find = FALSE;
				break;
			}
		}
		if (find) {
			psw->paint = hWnd;
			return;
		}
	}
	if (wls.size >= 4) {
		if (GetWindowLongPtr(wls.list[0], GWL_STYLE)   == 0x50000000 &&
			GetWindowLongPtr(wls.list[0], GWL_EXSTYLE) == 0x00000000 &&
			GetWindowLongPtr(wls.list[1], GWL_STYLE)   == 0x50000000 &&
			GetWindowLongPtr(wls.list[1], GWL_EXSTYLE) == 0x00000000 &&
			GetWindowLongPtr(wls.list[2], GWL_STYLE)   == 0x50000000 &&
			GetWindowLongPtr(wls.list[2], GWL_EXSTYLE) == 0x00000200 &&
			GetWindowLongPtr(wls.list[3], GWL_STYLE)   == 0x50000000 &&
			GetWindowLongPtr(wls.list[3], GWL_EXSTYLE) == 0x00000000) {
			psw->tools = hWnd;
			return;
		}
	}
	if (wls.size == 9) {
		psw->nav = hWnd;
	}
	else if (wls.size == 10) {
		psw->layers = hWnd;
	}
	else if (wls.size == 8) {
		psw->color = hWnd;
	}
	else if (wls.size == 17) {
		psw->top = hWnd;
		psw->zoom = wls.list[5];
		psw->rotate = wls.list[9];
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