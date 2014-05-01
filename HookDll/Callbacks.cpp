#include "stdafx.h"
#include "Callbacks.h"
#include "Shared.h"
#include "cmanipulationeventsink.h"
#include <manipulations.h>
#include <tpcshrd.h>
#include <manipulations_i.c>

typedef BOOL (WINAPI *pSetWindowFeedbackSetting)(HWND hwnd, FEEDBACK_TYPE feedback, DWORD dwFlags, UINT32 size, const VOID *configuration);

static std::vector<POINT> g_MSVectors;
void InitTouchWindow(HWND hwnd) {
	BOOL val = FALSE;
	SetProp(hwnd, MICROSOFT_TABLETPENSERVICE_PROPERTY,
		(HANDLE)(TABLET_DISABLE_FLICKS | TABLET_DISABLE_PRESSANDHOLD | TABLET_DISABLE_FLICKFALLBACKKEYS));

	HINSTANCE hInst = LoadLibrary(TEXT("user32.dll"));
	pSetWindowFeedbackSetting pfn = (pSetWindowFeedbackSetting)GetProcAddress(hInst, "SetWindowFeedbackSetting");
	if (pfn != NULL) {
		pfn(hwnd, FEEDBACK_TOUCH_CONTACTVISUALIZATION, 0, sizeof(BOOL), &val);
		pfn(hwnd, FEEDBACK_TOUCH_PRESSANDHOLD, 0, sizeof(BOOL), &val);
		pfn(hwnd, FEEDBACK_TOUCH_RIGHTTAP, 0, sizeof(BOOL), &val);
	}
	FreeLibrary(hInst);

	// Do not register touch window if you want WM_GESTURE
	RegisterTouchWindow(hwnd, TWF_FINETOUCH | TWF_WANTPALM);
	//RegisterPointerDeviceNotifications(GetSaiWindow(), TRUE);
}
void CheckFingerTap(DWORD tick, WORD x, WORD y) {
	// check tap
	for (int i = MAX_STATUS_FINGERS - 1; i >= 0; i --) {
		if (tick - gStatus.tgDownTicks[i] < gSettings.fingerTapInteval) {
			POINT pt; GetCursorPos(&pt);
			PostNotify(WM_USER_TOUCH, i, pt.x + pt.y * 0x10000);
			break;
		}
	}
}
void StrokeLine(HDC hdc, POINT ptFrom, POINT ptTo) {
	HPEN hpen = CreatePen(PS_SOLID, 5, RGB(0, 128, 255));
	HPEN hOld = (HPEN)SelectObject(hdc, hpen);
	MoveToEx(hdc, ptFrom.x, ptFrom.y, NULL);
	LineTo(hdc, ptTo.x, ptTo.y);
	DeleteObject(SelectObject(hdc, hOld));
}
void CheckEventTrigger(EVENT_TRIGGER *pe, double val) {
	int delta = 0;
	if (pe->msg) while (delta = ListIndex(pe, val))
		PostNotify(WM_USER_DEBUG + pe->msg, delta > 0 ? 0 : 1, pe->index);
}

void MsVectorReset() {
	g_MSVectors.clear();
}
void MsVectorReduce(int n) {
	std::vector<POINT> *ls = &g_MSVectors;
	int s = ls->size();
	if (s < n) return;

	POINT pb = (*ls)[s - n], pe = ls->back();
	double x = (pb.x + pe.x) * .5, y = (pb.y + pe.y) * .5,
		dx = pb.x - pe.x, dy = pb.y - pe.y, l = SQRT_SUM(dx, dy) + 1e-6;
	BOOL find = FALSE;
	for (int i = s - n; i < s && !find; i ++) {
		POINT pc = (*ls)[i];
		double d = fabs(dy * (pc.x - pb.x) - dx * (pc.y - pb.y)) / l,
			r = SQRT_SUM(pb.x - x, pb.y - y);
		find = (d > l * gSettings.mgDistanceIn || r > l * gSettings.mgRadiusIn);
	}
	if (!find) {
		ls->erase(ls->end() - n, ls->end());
		ls->push_back(pb);
		ls->push_back(pe);
	}
}
void MsVectorAdd(POINT pt) {
	g_MSVectors.push_back(pt);
	MsVectorReduce(gSettings.mgPointCount);
}
void MsVectorToString() {
	for (int n = gSettings.mgPointCount - 1; n >= 3; n --)
		MsVectorReduce(n);
	int j = 0;
	std::vector<POINT> *ls = &g_MSVectors;
	TCHAR *st = gStatus.mgVectorStr;
	for (std::vector<POINT>::iterator i = ls->begin(); i < ls->end() - 1 && j < MAX_VECTOR_LENGTH; i ++) {
		std::vector<POINT>::iterator b = i, e = i + 1;
		double k = fabs((b->y - e->y) / (b->x - e->x + 1e-6));
		TCHAR c = j > 0 ? st[j-1] : 0;
		if (k > gSettings.mgSlope) c = e->y > b->y ? L'd' : L'u';
		else if (k < 1.0/gSettings.mgSlope) c = e->x > b->x ? L'r' : L'l';
		else if (e->x > b->x) c = e->y > b->y ? L'R' : L'U';
		else c = e->y > b->y ? L'D' : L'L';
		if (j == 0 || st[j-1] != c)
			st[j ++] = c;
	}
	if (j < MAX_VECTOR_LENGTH)
		st[j] = 0;
}
void MsVectorToEmpty() {
	gStatus.mgVectorStr[0] = 0;
}

void TouchGestureKeep(DWORD n, long x, long y, long s, long r) {
	gStatus.tgScale = s;
	gStatus.tgRotate = r;
	if (n == 1) {
		if (!gStatus.tgState && SQUA_SUM(x, y) > SQUA(10))
			PostNotify(WM_USER_TOUCH, (gStatus.tgState = n) + 0x10000, x + y * 0x10000);
	}
	else if (n == 2) {
		if (!gStatus.tgState && (s < 100-5 || s > 100+5 || r < -5 || r > 5))
			PostNotify(WM_USER_TOUCH, (gStatus.tgState = n) + 0x10000, x + y * 0x10000);
		if (gStatus.tgState == n) {
			CheckEventTrigger(&gSettings.evtZoom, s);
			CheckEventTrigger(&gSettings.evtRotate, r);
		}
	}
}
void TouchGestureEnd() {
	if (gSettings.dragKey.enabled) {
		SHORTCUT_KEY *pk = &gSettings.dragKey;
		pk->enabled = FALSE;
		SimulateMouse(0, 0, 0, MOUSEEVENTF_LEFTUP);
		SimulateShortcut(pk, FALSE);
	}
	gSettings.evtZoom.msg = 0;
	gSettings.evtRotate.msg = 0;
	gStatus.tgState = 0;
}

void MouseGestureBegin(DWORD tick) {
	gStatus.mgTick = tick;
	gStatus.mgState = 0;
	gStatus.mgBeginPos = gStatus.penHoverPos;
	MsVectorReset();
}
void MouseGestureKeep(UINT message) {
	if (message == WM_MOUSEMOVE) {
		static POINT strokePt;
		MsVectorAdd(gStatus.penHoverPos);
		if (gStatus.mgState == 0) {
			if (SQUA_SUM(gStatus.mgBeginPos.x - gStatus.penHoverPos.x, gStatus.mgBeginPos.y - gStatus.penHoverPos.y) >
				(LONG)SQUA(gSettings.mgEnableDistance)) {
				strokePt = gStatus.mgBeginPos;
				gStatus.mgState = WM_MOUSEMOVE;
			}
		}
		else if (gStatus.mgState == WM_MOUSEMOVE) {
			HDC hdc = GetDC(NULL);
			StrokeLine(hdc, strokePt, gStatus.penHoverPos);
			strokePt = gStatus.penHoverPos;
			ReleaseDC(NULL, hdc);
		}
		else if (gStatus.mgState == WM_LBUTTONDOWN) {
			CheckEventTrigger(&gSettings.evtOffsetX, gStatus.penHoverPos.x);
			CheckEventTrigger(&gSettings.evtOffsetY, gStatus.penHoverPos.y);
		}
	}
	else if (message == WM_LBUTTONDOWN) {
		POINT pt = gStatus.penHoverPos;
		int lppos = pt.x + pt.y * 0x10000;
		InvalidateRect(WindowFromPoint(pt), NULL, FALSE);
		if (gStatus.mgState == 0) {
			MsVectorToEmpty();
			PostNotify(WM_USER_GESTURE, 1, lppos);
		}
		else if (gStatus.mgState == WM_MOUSEMOVE) {
			MsVectorToString();
			PostNotify(WM_USER_GESTURE, 1, lppos);
		}
		gStatus.mgState = WM_LBUTTONDOWN;
		// move the cursor to current position
		SimulateMouse(0, 5, 0, MOUSEEVENTF_MOVE);
		SimulateMouse(0, -5, 0, MOUSEEVENTF_MOVE);
	}
}
void MouseGestureEnd(DWORD tick, HWND hwnd) {
	POINT pt = gStatus.penHoverPos;
	int lppos = pt.x + pt.y * 0x10000;
	if (gStatus.mgState == 0 && tick - gStatus.mgTick < gSettings.vkTimeout &&
			IsPainterWindow(hwnd)) {
		MsVectorToEmpty();
		PostNotify(WM_USER_GESTURE, 0, lppos);
	}
	else if (gStatus.mgState == WM_MOUSEMOVE) {
		MsVectorToString();
		PostNotify(WM_USER_GESTURE, 0, lppos);
	}
	InvalidateRect(WindowFromPoint(gStatus.penHoverPos), NULL, FALSE);
	gStatus.mgTick = 0;

	gSettings.evtOffsetX.msg = gSettings.evtOffsetY.msg = 0;
	if (gSettings.dragKey.enabled) {
		SHORTCUT_KEY *pk = &gSettings.dragKey;
		pk->enabled = FALSE;
		SimulateMouse(0, 0, 0, MOUSEEVENTF_LEFTUP);
		SimulateShortcut(pk, FALSE);
	}
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		MSG *msg = (MSG *)lParam;
		DWORD tick = GetTickCount();

		// Init
		if (!IsTouchWindow(msg->hwnd, 0))
			InitTouchWindow(msg->hwnd);
		// setup touch lock timeout
		if (msg->message == WM_PEN_HOVER_UNKNOWN || msg->message == WT_PACKET)
			gStatus.penHoverTick = tick;
		BOOL bEnableTouch = !gSettings.lockTouch &&
			(tick - gStatus.penHoverTick > TIMEOUT_TOUCH_ENABLE_AFTER_PEN_HOVER);

		/*
		 * Block events from touch
		 */
		if (msg->message == WM_MOUSEMOVE ||
			msg->message == WM_NCHITTEST || msg->message == WM_NCMOUSEMOVE ||
			msg->message == WM_NCLBUTTONDOWN ||
			msg->message == WM_SETCURSOR || msg->message == WM_MOUSEWHEEL ||
			msg->message == WM_LBUTTONUP || msg->message == WM_LBUTTONDOWN ||
			msg->message == WM_RBUTTONUP || msg->message == WM_RBUTTONDOWN ||
			msg->message == WM_LBUTTONDBLCLK || msg->message == WM_RBUTTONDBLCLK ||
			msg->message == WM_KEYDOWN || msg->message == WM_KEYUP ||
			msg->message == WM_VSCROLL || msg->message == WM_HSCROLL) {
			if (msg->message == WM_MOUSEMOVE && gSettings.dragKey.enabled && gStatus.tgState == 1)
				; // pass
			else if ((!bEnableTouch ||
					IsPainterWindow(msg->hwnd)) &&
				(GetMessageExtraInfo() & EVENTF_FROMTOUCH) == EVENTF_FROMTOUCH)
				msg->message += WM_USER;
		}
		// remember mouse position
		if (msg->message == WM_MOUSEMOVE)
			gStatus.penHoverPos = msg->pt;

		/*
		 * Test if right mouse is down
		 */
		if ((msg->message == WM_LBUTTONDOWN || msg->message == WM_LBUTTONUP ||
				msg->message == WM_RBUTTONDOWN || msg->message == WM_RBUTTONUP) &&
			GetMessageExtraInfo() != LLMHF_INJECTED) {
			if (msg->message == WM_LBUTTONDOWN || msg->message == WM_LBUTTONUP)
				gStatus.isLeftDown = msg->message == WM_LBUTTONDOWN;
			else if (msg->message == WM_RBUTTONDOWN || msg->message == WM_RBUTTONUP)
				gStatus.isRightDown = msg->message == WM_RBUTTONDOWN;

			// block right mouse button
			if ((msg->message == WM_RBUTTONDOWN || msg->message == WM_RBUTTONUP) &&
					IsPainterWindow(msg->hwnd))
				msg->message += WM_USER;
		}

		/*
		 * Process mouse gestures
		 */
		static DWORD releaseTick = tick;
		if (gStatus.isRightDown && !gStatus.mgTick)
			MouseGestureBegin(tick);
		if (gStatus.mgTick) {
			MouseGestureKeep(msg->message);
			// block unwanted events
			if (msg->message == WM_MOUSEMOVE && gSettings.dragKey.enabled)
				; // pass
			else if ((msg->message == WM_MOUSEMOVE||
					msg->message == WM_LBUTTONDOWN ||
					msg->message == WM_LBUTTONUP ||
					msg->message == WM_PEN_HOVER_UNKNOWN) &&
				GetMessageExtraInfo() != LLMHF_INJECTED)
				msg->message += WM_USER;
		}
		if (gStatus.isRightDown || gStatus.isLeftDown)
			releaseTick = tick + TIMEOUT_MOUSE_GESTURE_FINISH_DELAY;
		else if (gStatus.mgTick && tick > releaseTick)
			MouseGestureEnd(tick, msg->hwnd);


		/*
		 * Process touch gestures
		 */
		DWORD n = gStatus.tgFingers;
		if (msg->message == WM_GESTURE_PROC) {
			SHORT x = LOWORD(msg->lParam) - 0x8000, y = HIWORD(msg->lParam) - 0x8000,
				s = LOWORD(msg->wParam) - 0x8000, r = HIWORD(msg->wParam) - 0x8000;
			if (bEnableTouch)
				TouchGestureKeep(n, x, y, s, r);
		}
		if (msg->message == WM_GESTURE_DOWN) {
			if (n > 0 && n <= MAX_STATUS_FINGERS)
				gStatus.tgDownTicks[n - 1] = tick;
		}
		else if (msg->message == WM_GESTURE_UP) {
			if (n == 0) {
				if (bEnableTouch && !gStatus.tgState)
					CheckFingerTap(tick, LOWORD(msg->lParam), HIWORD(msg->lParam));
				TouchGestureEnd();
			}
			if (n >= 0 && n < MAX_STATUS_FINGERS)
				gStatus.tgUpTicks[n] = tick;
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static IManipulationProcessor* g_pIManipProc = NULL;
static CManipulationEventSink* g_pManipulationEventSink = NULL;
LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		DWORD tick = GetTickCount();
		CWPRETSTRUCT *cs = (CWPRETSTRUCT *)lParam;

		// send touch events to g_pIManipProc, and it will post WM_GESTURE_PROC back when processing
		if (cs->message == WM_TOUCH) {
			if (g_pIManipProc == NULL || g_pManipulationEventSink == NULL) {
				CoInitialize(0);
				if (g_pIManipProc == NULL)
					CoCreateInstance(CLSID_ManipulationProcessor, NULL, CLSCTX_INPROC_SERVER,
						IID_IUnknown, (VOID**)(&g_pIManipProc));
				if (g_pIManipProc != NULL)
					g_pManipulationEventSink = new CManipulationEventSink(g_pIManipProc, gStatus.targetThread, WM_GESTURE_PROC);
			}
			if (g_pIManipProc != NULL && g_pManipulationEventSink != NULL) {
				HTOUCHINPUT hTi = (HTOUCHINPUT)cs->lParam;
				UINT cTi = LOWORD(cs->wParam);
				TOUCHINPUT *pTi = new TOUCHINPUT[cTi];
				if (GetTouchInputInfo(hTi, cTi, pTi, sizeof(TOUCHINPUT))) {
					for (UINT i = 0; i < cTi; i ++) {
						if (pTi[i].dwFlags & TOUCHEVENTF_DOWN) {
							gStatus.tgFingers = cTi;
							g_pIManipProc->ProcessDownWithTime(pTi[i].dwID, static_cast<FLOAT>(pTi[i].x), static_cast<FLOAT>(pTi[i].y), tick);
							PostThreadMessage(gStatus.targetThread, WM_GESTURE_DOWN, (WPARAM)cs->hwnd, MAKELONG(pTi[i].x/100, pTi[i].y/100));
						}
						if (pTi[i].dwFlags & TOUCHEVENTF_UP) {
							gStatus.tgFingers --;
							g_pIManipProc->ProcessUpWithTime(pTi[i].dwID, static_cast<FLOAT>(pTi[i].x), static_cast<FLOAT>(pTi[i].y), tick);
							PostThreadMessage(gStatus.targetThread, WM_GESTURE_UP, (WPARAM)cs->hwnd, MAKELONG(pTi[i].x/100, pTi[i].y/100));
							if (!gStatus.tgFingers)
								g_pIManipProc->CompleteManipulation();
						}
						if (pTi[i].dwFlags & TOUCHEVENTF_MOVE) {
							gStatus.tgWindow = cs->hwnd;
							g_pIManipProc->ProcessMoveWithTime(pTi[i].dwID, static_cast<FLOAT>(pTi[i].x), static_cast<FLOAT>(pTi[i].y), tick);
						}
					}
					CloseTouchInputHandle(hTi);
				}
			}
		}

		// A Dirty Fix for pen focus issue
		// When leaving painter window, send an extra mouse move event
		static DWORD painterLeaveTick = 0;
		if (cs->message == WM_MOUSELEAVE &&
			IsPainterWindow(cs->hwnd)) {
			if (tick - painterLeaveTick > gSettings.painterLeaveTimeout) {
				SimulateMouse(0, 0, 0, MOUSEEVENTF_MOVE);
				PostMessage(cs->hwnd, 0x0ff2, 0, 0);
			}
			painterLeaveTick = tick;
		}
		// Once cursor enter again, simulate a "ctrl" key
		if (cs->message == WM_SETCURSOR &&
			painterLeaveTick > 0 && tick - painterLeaveTick > gSettings.painterLeaveTimeout &&
			IsPainterWindow(cs->hwnd)) {
			SimulateKey(VK_CONTROL, 0);
			SimulateKey(VK_CONTROL, KEYEVENTF_KEYUP);
			painterLeaveTick = 0;
		}

		// Clean up
		if (cs->message == WM_USER_QUIT) {
			RemoveProp(cs->hwnd, MICROSOFT_TABLETPENSERVICE_PROPERTY);
			UnregisterTouchWindow(cs->hwnd);
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

BOOL CALLBACK SendQuitMsgProc(HWND hWnd, LPARAM lParam) {
	SendMessage(hWnd, WM_USER_QUIT, WM_QUIT, 0);
	EnumChildWindows(hWnd, SendQuitMsgProc, NULL);
	return TRUE;
}