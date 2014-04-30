#include "stdafx.h"
#include "Callbacks.h"
#include "Shared.h"
#include "cmanipulationeventsink.h"
#include <manipulations.h>
#include <tpcshrd.h>
#include <manipulations_i.c>

static IManipulationProcessor* g_pIManipProc = NULL;
static CManipulationEventSink* g_pManipulationEventSink = NULL;
static std::vector<POINT> g_MSVectors;

typedef BOOL (WINAPI *pSetWindowFeedbackSetting)(HWND hwnd, FEEDBACK_TYPE feedback, DWORD dwFlags, UINT32 size, const VOID *configuration);

void ResetVector() {
	g_MSVectors.clear();
}
void ReducePoint(int n) {
	int s = g_MSVectors.size();
	if (s < n) return;

	POINT pb = g_MSVectors[s - n], pe = g_MSVectors.back();
	double x = (pb.x + pe.x) * .5, y = (pb.y + pe.y) * .5,
		dx = pb.x - pe.x, dy = pb.y - pe.y, l = SQRT_SUM(dx, dy) + 1e-6;
	BOOL find = FALSE;
	for (int i = s - n; i < s && !find; i ++) {
		POINT pc = g_MSVectors[i];
		double d = fabs(dy * (pc.x - pb.x) - dx * (pc.y - pb.y)) / l,
			r = SQRT_SUM(pb.x - x, pb.y - y);
		find = (d > l * gSettings.mgDistanceIn || r > l * gSettings.mgRadiusIn);
	}
	if (!find) {
		g_MSVectors.erase(g_MSVectors.end() - n, g_MSVectors.end());
		g_MSVectors.push_back(pb);
		g_MSVectors.push_back(pe);
	}
}
void AddPoint(POINT pt) {
	g_MSVectors.push_back(pt);
	ReducePoint(gSettings.mgPointCount);
}
void GetVector() {
	for (int n = gSettings.mgPointCount - 1; n >= 3; n --)
		ReducePoint(n);
	int j = 0;
	for (std::vector<POINT>::iterator i = g_MSVectors.begin(); i < g_MSVectors.end() - 1 && j < MAX_VECTOR_LENGTH; i ++) {
		std::vector<POINT>::iterator b = i, e = i + 1;
		double k = fabs((b->y - e->y) / (b->x - e->x + 1e-6));
		TCHAR c = j > 0 ? gStatus.vectorStr[j-1] : 0;
		if (k > gSettings.mgSlope) c = e->y > b->y ? L'd' : L'u';
		else if (k < 1.0/gSettings.mgSlope) c = e->x > b->x ? L'r' : L'l';
		else if (e->x > b->x) c = e->y > b->y ? L'R' : L'U';
		else c = e->y > b->y ? L'D' : L'L';
		if (j == 0 || gStatus.vectorStr[j-1] != c)
			gStatus.vectorStr[j ++] = c;
	}
	if (j < MAX_VECTOR_LENGTH)
		gStatus.vectorStr[j] = 0;
}
void GetVectorEmpty() {
	gStatus.vectorStr[0] = 0;
}

void KeepGesture(long x, long y, long s, long r) {
	if (gStatus.gestureId == GID_PAN) {
		if (gStatus.panVkState != 0)
			SimulateKey((WORD)gStatus.panVkState, 0);
	}
	else if (gStatus.gestureId == GID_ZOOM) {
		int i = FindInArray(gSettings.zoomArr, MAX_SETTING_STEPS, s);
		if (gStatus.zoomIndex != i) {
			SimulateKey(gStatus.zoomIndex > i ? VK_NEXT : VK_PRIOR, 0);
			SimulateKey(gStatus.zoomIndex > i ? VK_NEXT : VK_PRIOR, KEYEVENTF_KEYUP);
			gStatus.zoomIndex > i ? gStatus.zoomIndex-- : gStatus.zoomIndex++;
		}
	}
	else if (gStatus.gestureId == GID_ROTATE) {
		int i = FindInArray(gSettings.rotateArr, MAX_SETTING_STEPS, r);
		if (gStatus.rotateIndex != i) {
			SimulateKey(gStatus.rotateIndex > i ? VK_DELETE : VK_END, 0);
			SimulateKey(gStatus.rotateIndex > i ? VK_DELETE : VK_END, KEYEVENTF_KEYUP);
			gStatus.rotateIndex > i ? gStatus.rotateIndex-- : gStatus.rotateIndex++;
		}
	}
}
void ChangeGesture(DWORD newState, long param) {
	DWORD oldState = gStatus.gestureId;
	if (oldState == newState)
		return;
	// close old state
	if (oldState == GID_PAN) {
		SimulateMouse(0, 0, 0, MOUSEEVENTF_LEFTUP);
		if (gStatus.panVkState != 0)
			SimulateKey((WORD)gStatus.panVkState, KEYEVENTF_KEYUP);
		gStatus.panVkState = 0;
	}
	else if (oldState == GID_ZOOM) {
	}
	else if (oldState == GID_ROTATE) {
	}
//	if (g_dwGestureState != GID_BEGIN)
//		LogText(TEXT("gesture %x off\r\n"), g_dwGestureState);

	gStatus.gestureId = newState;
	// begin new
	if (newState == GID_PAN) {
		gStatus.panVkState = gSettings.panVkCode;
		if (gStatus.panVkState != 0)
			SimulateKey((WORD)gStatus.panVkState, 0);
		SimulateMouse(0, 0, 0, MOUSEEVENTF_MOVE);
		SimulateMouse(0, 0, 0, MOUSEEVENTF_LEFTDOWN);
	}
	else if (newState == GID_ZOOM) {
		gStatus.zoomIndex = FindInArray(gSettings.zoomArr, MAX_SETTING_STEPS, param);
	}
	else if (newState == GID_ROTATE) {
		gStatus.rotateIndex = FindInArray(gSettings.rotateArr, MAX_SETTING_STEPS, param);
	}
//	if (g_dwGestureState != GID_BEGIN)
//		LogText(TEXT("gesture %x on (%d/0x%x)\r\n"), g_dwGestureState, param, param);
}
void HandleGesture(DWORD tick, SHORT x, SHORT y, SHORT s, SHORT r) {
		// two finger gesture (zoom, rotate)
		if (gStatus.fingerCount == 2 &&
			tick - gStatus.fingerDownTick[2] > 100 &&
			tick - gStatus.fingerDownTick[3] > 800 &&
			tick - gStatus.fingerDownTick[4] > 800 &&
			1) {
			if ((r > gSettings.rotateTriMin && r < gSettings.rotateTriMax) &&
				(s < gSettings.zoomTriMin || s > gSettings.zoomTriMax))
				ChangeGesture(GID_ZOOM, s);
			else if ((r < gSettings.rotateTriMin || r > gSettings.rotateTriMax) &&
				(s > gSettings.zoomTriMin && s < gSettings.zoomTriMax))
				ChangeGesture(GID_ROTATE, r);
		}
		if ((gStatus.gestureId == GID_ZOOM || gStatus.gestureId == GID_ROTATE) &&
			(gStatus.fingerCount != 2 || SQUA_SUM(x, y) > (LONG)SQUA(gSettings.guestureCancelDistance)))
			ChangeGesture(GID_BEGIN, 0);

		// one finger gesture (pan)
		if (gStatus.fingerCount == 1 &&
			tick - gStatus.fingerDownTick[1] > 50 &&
			tick - gStatus.fingerDownTick[2] > 800 &&
			tick - gStatus.fingerDownTick[3] > 800 &&
			gStatus.gestureId != GID_PAN &&
			SQUA_SUM(x, y) > (LONG)SQUA(gSettings.panTriggerDistance)) {
			POINT pt; GetCursorPos(&pt);
			if (IsPainterWindow(WindowFromPoint(pt))) {
				ChangeGesture(GID_PAN, 0);
			}
		}
		if (gStatus.fingerCount != 1 && gStatus.gestureId == GID_PAN)
			ChangeGesture(GID_BEGIN, 0);

		// cancel all gesture if more than 2 fingers was on
		if (tick - gStatus.fingerDownTick[3] < gSettings.guestureEnableTimeout)
			ChangeGesture(GID_BEGIN, 0);
		else
			KeepGesture(x, y, s, r);
}

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
		if (tick - gStatus.fingerDownTick[i] < gSettings.fingerTapInteval) {
			POINT pt; GetCursorPos(&pt);
			PostNotify(WM_USER_FINGERTAP, i, pt.x + pt.y * 0x10000);
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

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {

	if (nCode == HC_ACTION) {
		MSG *msg = (MSG *)lParam;
		DWORD tick = GetTickCount();

		// Init
		if (!IsTouchWindow(msg->hwnd, 0))
			InitTouchWindow(msg->hwnd);
		// setup touch lock timeout
		if (msg->message == UNKNOWN_PEN_MSG || msg->message == WT_PACKET)
			gStatus.penHoverTick = tick;
		gStatus.bEnableTouch = !gSettings.lockTouch &&
			(tick - gStatus.penHoverTick > gSettings.touchEnableTimeout);

		/*
		 * Block events from touch
		 */
		if ((msg->message == WM_MOUSEMOVE && gStatus.gestureId != GID_PAN) ||
			msg->message == WM_NCHITTEST || msg->message == WM_NCMOUSEMOVE ||
			msg->message == WM_NCLBUTTONDOWN ||
			msg->message == WM_SETCURSOR || msg->message == WM_MOUSEWHEEL ||
			msg->message == WM_LBUTTONUP || msg->message == WM_LBUTTONDOWN ||
			msg->message == WM_RBUTTONUP || msg->message == WM_RBUTTONDOWN ||
			msg->message == WM_LBUTTONDBLCLK || msg->message == WM_RBUTTONDBLCLK ||
			msg->message == WM_KEYDOWN || msg->message == WM_KEYUP ||
			msg->message == WM_VSCROLL || msg->message == WM_HSCROLL) {
			if ((!gStatus.bEnableTouch ||
					gStatus.gestureId != GID_BEGIN ||
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
		// check if CTRL or ALT is down
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

			// trigger on
			if (gStatus.isRightDown && !gStatus.vkDownTick) {
				gStatus.vkDownTick = tick;
				gStatus.vkStateId = 0;
				gStatus.vkDownPos = gStatus.penHoverPos;
				ResetVector();
			}
			else if (!gStatus.isRightDown && !gStatus.isLeftDown && gStatus.vkDownTick) {
				POINT pt = gStatus.penHoverPos;
				int lppos = pt.x + pt.y * 0x10000;
				if (gStatus.vkStateId == 0 && tick - gStatus.vkDownTick < gSettings.vkTimeout &&
						IsPainterWindow(msg->hwnd)) {
					GetVectorEmpty();
					PostNotify(WM_USER_GESTURE, 0, lppos);
				}
				else if (gStatus.vkStateId == WM_MOUSEMOVE) {
					GetVector();
					PostNotify(WM_USER_GESTURE, 0, lppos);
				}
				InvalidateRect(WindowFromPoint(gStatus.penHoverPos), NULL, FALSE);
				gStatus.vkDownTick = 0;

				gSettings.mgStepMsg = 0;
				if (gSettings.mgDrag.enabled) {
					DRAG_KEY *pdk = &gSettings.mgDrag;
					pdk->enabled = FALSE;
					SimulateMouse(0, 0, 0, MOUSEEVENTF_LEFTUP);
					if (pdk->vk)
						SimulateKey(pdk->vk, KEYEVENTF_KEYUP);
					if (pdk->ctrl)
						SimulateKey(VK_CONTROL, KEYEVENTF_KEYUP);
					if (pdk->shift)
						SimulateKey(VK_SHIFT, KEYEVENTF_KEYUP);
					if (pdk->alt)
						SimulateKey(VK_MENU, KEYEVENTF_KEYUP);
				}
			}
		}

		/*
		 * Process virtual key events
		 */
		if (gStatus.vkDownTick) {
			if (msg->message == WM_MOUSEMOVE) {
				AddPoint(gStatus.penHoverPos);
				if (gStatus.vkStateId == 0) {
					if (SQUA_SUM(gStatus.vkDownPos.x - gStatus.penHoverPos.x, gStatus.vkDownPos.y - gStatus.penHoverPos.y) >
						(LONG)SQUA(gSettings.mgEnableDistance)) {
						gStatus.vkStrokePos = gStatus.vkDownPos;
						gStatus.vkStateId = WM_MOUSEMOVE;
					}
				}
				else if (gStatus.vkStateId == WM_MOUSEMOVE) {
					HDC hdc = GetDC(NULL);
					StrokeLine(hdc, gStatus.vkStrokePos, gStatus.penHoverPos);
					gStatus.vkStrokePos = gStatus.penHoverPos;
					ReleaseDC(NULL, hdc);
				}
				else if (gStatus.vkStateId == WM_LBUTTONDOWN) {
					if (gSettings.mgStepMsg) {
						int delta = 0;
						DWORD ms = WM_USER_DEBUG + gSettings.mgStepMsg;
						while (delta = ListIndex(&gSettings.mgStepX, gStatus.penHoverPos.x))
							PostNotify(ms, 0+(delta > 0 ? 0 : 1), gSettings.mgStepX.index);
						while (delta = ListIndex(&gSettings.mgStepY, gStatus.penHoverPos.y))
							PostNotify(ms, 2+(delta > 0 ? 0 : 1), gSettings.mgStepY.index);
					}
				}
			}
			else if (msg->message == WM_LBUTTONDOWN) {
				POINT pt = gStatus.penHoverPos;
				int lppos = pt.x + pt.y * 0x10000;
				InvalidateRect(WindowFromPoint(pt), NULL, FALSE);
				if (gStatus.vkStateId == 0) {
					GetVectorEmpty();
					PostNotify(WM_USER_GESTURE, 1, lppos);
				}
				else if (gStatus.vkStateId == WM_MOUSEMOVE) {
					GetVector();
					PostNotify(WM_USER_GESTURE, 1, lppos);
				}
				gStatus.vkStateId = WM_LBUTTONDOWN;
				// move the cursor to current position
				SimulateMouse(0, 5, 0, MOUSEEVENTF_MOVE);
				SimulateMouse(0, -5, 0, MOUSEEVENTF_MOVE);
			}

			// block
			if (((msg->message == WM_MOUSEMOVE && !gSettings.mgDrag.enabled) ||
					msg->message == WM_LBUTTONDOWN ||
					msg->message == WM_LBUTTONUP ||
					msg->message == UNKNOWN_PEN_MSG) &&
					GetMessageExtraInfo() != LLMHF_INJECTED)
				msg->message += WM_USER;
		}

		/*
		 * Process gesture messages
		 */
		if (msg->message == WM_GESTURE_PROC) {
			SHORT x = LOWORD(msg->lParam) - 0x8000, y = HIWORD(msg->lParam) - 0x8000,
				s = LOWORD(msg->wParam) - 0x8000, r = HIWORD(msg->wParam) - 0x8000;
			gStatus.bEnableTouch ? HandleGesture(tick, x, y, s, r) : ChangeGesture(GID_BEGIN, 0);
		}
		DWORD n = gStatus.fingerCount;
		if (msg->message == WM_GESTURE_DOWN) {
			if (n > 0 && n <= MAX_STATUS_FINGERS)
				gStatus.fingerDownTick[n - 1] = tick;
		}
		else if (msg->message == WM_GESTURE_UP) {
			if (n == 0) {
				// give up all gesture beacuse there are no fingers touching now
				ChangeGesture(GID_BEGIN, 0);
				g_pIManipProc->CompleteManipulation();
				if (gStatus.bEnableTouch)
					CheckFingerTap(tick, LOWORD(msg->lParam), HIWORD(msg->lParam));
			}
			if (n >= 0 && n < MAX_STATUS_FINGERS)
				gStatus.fingerUpTick[n] = tick;
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

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
					g_pManipulationEventSink = new CManipulationEventSink(g_pIManipProc, gStatus.threadId, WM_GESTURE_PROC);
			}
			if (g_pIManipProc != NULL && g_pManipulationEventSink != NULL) {
				HTOUCHINPUT hTi = (HTOUCHINPUT)cs->lParam;
				UINT cTi = LOWORD(cs->wParam);
				TOUCHINPUT *pTi = new TOUCHINPUT[cTi];
				if (GetTouchInputInfo(hTi, cTi, pTi, sizeof(TOUCHINPUT))) {
					for (UINT i = 0; i < cTi; i ++) {
						if (pTi[i].dwFlags & TOUCHEVENTF_DOWN) {
							gStatus.fingerCount = cTi;
							g_pIManipProc->ProcessDownWithTime(pTi[i].dwID, static_cast<FLOAT>(pTi[i].x), static_cast<FLOAT>(pTi[i].y), tick);
							PostThreadMessage(gStatus.threadId, WM_GESTURE_DOWN, gStatus.fingerCount, MAKELONG(pTi[i].x/100, pTi[i].y/100));
						}
						if (pTi[i].dwFlags & TOUCHEVENTF_UP) {
							gStatus.fingerCount --;
							g_pIManipProc->ProcessUpWithTime(pTi[i].dwID, static_cast<FLOAT>(pTi[i].x), static_cast<FLOAT>(pTi[i].y), tick);
							PostThreadMessage(gStatus.threadId, WM_GESTURE_UP, gStatus.fingerCount, MAKELONG(pTi[i].x/100, pTi[i].y/100));
						}
						if (pTi[i].dwFlags & TOUCHEVENTF_MOVE)
							g_pIManipProc->ProcessMoveWithTime(pTi[i].dwID, static_cast<FLOAT>(pTi[i].x), static_cast<FLOAT>(pTi[i].y), tick);
					}
					CloseTouchInputHandle(hTi);
				}
			}
		}

		// A Dirty Fix for pen focus issue
		// When leaving painter window, send an extra mouse move event
		if (cs->message == WM_MOUSELEAVE &&
			IsPainterWindow(cs->hwnd)) {
			if (tick - gStatus.painterLeaveTick > gSettings.painterLeaveTimeout) {
				SimulateMouse(0, 0, 0, MOUSEEVENTF_MOVE);
				PostMessage(cs->hwnd, 0x0ff2, 0, 0);
			}
			gStatus.painterLeaveTick = tick;
		}
		// Once cursor enter again, simulate a "ctrl" key
		if (cs->message == WM_SETCURSOR &&
			gStatus.painterLeaveTick > 0 && tick - gStatus.painterLeaveTick > gSettings.painterLeaveTimeout &&
			IsPainterWindow(cs->hwnd)) {
			SimulateKey(VK_CONTROL, 0);
			SimulateKey(VK_CONTROL, KEYEVENTF_KEYUP);
			gStatus.painterLeaveTick = 0;
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