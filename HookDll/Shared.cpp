#include "stdafx.h"
#include "Shared.h"

#pragma data_seg("Share")

SETTINGS gSettings = {
	FALSE,
	{0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
};

STATUS gStatus = {
	0,
	0,

	0,
	{0, 0},

	GID_BEGIN,
	0,
	NULL,
	0,
	0,
	{0},
	{0},

	FALSE,
	FALSE,
	0,
	0,
	{0, 0},
	0,
	{0},
};

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

BOOL IsPainterWindow(HWND hWnd) {
	LONG lStyle = WS_CHILDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	return (GetWindowLongPtr(hWnd, GWL_STYLE) & lStyle) == lStyle;
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
