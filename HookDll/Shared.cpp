#include "stdafx.h"
#include "Shared.h"

#pragma data_seg("Share")

SETTINGS gSettings = {
	FALSE,

	500,
	800,
	500,

	VK_SPACE,
	500,
	200,

	120,
	35,
	8,
	2.0,
	0.20,
	0.55,

	{0},
	{0, 0, 0},
	{0, 0, 0},

	10,
	80,
	90,
	110,
	{-1000, 5, 10, 15, 25, 35, 45, 55, 65, 75, 85, 100, 115, 125, 135, 145, 155, 165, 175, 185, 195, 200, 2000},
	-10,
	10,
	{-1000, -180, -150, -120, -90, -85, -75, -65, -55, -45, -35, -25, -15, 0, 15, 25, 35, 45, 55, 65, 75, 85, 90, 120, 150, 180, 1000},
};

STATUS gStatus = {
	0,
	0,

	FALSE,

	0,
	{0, 0},

	GID_BEGIN,
	0,
	{0},
	{0},

	FALSE,
	FALSE,
	0,
	0,
	{0, 0},
	{0, 0},

	0,

	0,
	0,
	0,

	{0},
};

#pragma data_seg()
#pragma comment(linker, "/Section:Share,rws")

int FindInArray(double *arr, int size, double value) {
	int i;
	for(i = 0; i < size - 1 && !(arr[i] <= value && arr[i+1] > value); i ++);
	return i;
}

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
