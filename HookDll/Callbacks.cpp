#include "stdafx.h"
#include "Callbacks.h"
#include "Shared.h"
#include "cmanipulationeventsink.h"
#include <manipulations.h>
#include <tpcshrd.h>
#include <manipulations_i.c>

static IManipulationProcessor* g_pIManipProc = NULL;
static CManipulationEventSink* g_pManipulationEventSink = NULL;

typedef BOOL (WINAPI *pSetWindowFeedbackSetting)(HWND hwnd, FEEDBACK_TYPE feedback, DWORD dwFlags, UINT32 size, const VOID *configuration);

void KeepGesture(long x, long y, long s, long r) {
	if (gStatus.gestureId == GID_PAN) {
		if (gSettings.panVkCode != 0)
			SimulateKey((WORD)gSettings.panVkCode, 0);
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
		if (gSettings.panVkCode != 0)
			SimulateKey((WORD)gSettings.panVkCode, KEYEVENTF_KEYUP);
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
		if (gSettings.panVkCode != 0)
			SimulateKey((WORD)gSettings.panVkCode, 0);
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
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {

	if (nCode == HC_ACTION) {
		MSG *msg = (MSG *)lParam;
		DWORD tick = GetTickCount();

		// Init
		if (!IsTouchWindow(msg->hwnd, 0)) {
			BOOL val = FALSE;
			SetProp(msg->hwnd, MICROSOFT_TABLETPENSERVICE_PROPERTY,
				(HANDLE)(TABLET_DISABLE_FLICKS | TABLET_DISABLE_PRESSANDHOLD | TABLET_DISABLE_FLICKFALLBACKKEYS));

			HINSTANCE hInst = LoadLibrary(TEXT("user32.dll"));
			pSetWindowFeedbackSetting pfn = (pSetWindowFeedbackSetting)GetProcAddress(hInst, "SetWindowFeedbackSetting");
			if (pfn != NULL) {
				pfn(msg->hwnd, FEEDBACK_TOUCH_CONTACTVISUALIZATION, 0, sizeof(BOOL), &val);
				pfn(msg->hwnd, FEEDBACK_TOUCH_PRESSANDHOLD, 0, sizeof(BOOL), &val);
				pfn(msg->hwnd, FEEDBACK_TOUCH_RIGHTTAP, 0, sizeof(BOOL), &val);
			}
			FreeLibrary(hInst);

			// Do not register touch window if you want WM_GESTURE
			RegisterTouchWindow(msg->hwnd, TWF_FINETOUCH | TWF_WANTPALM);
//			RegisterPointerDeviceNotifications(GetSaiWindow(), TRUE);
		}

		// setup touch lock timeout
		if (msg->message == 0x0ff2) {
			// Do not use msg->time!
			gStatus.penHoverTick = tick;
			GetCursorPos(&gStatus.penHoverPos);
		}
		BOOL bEnableTouch = (tick - gStatus.penHoverTick > gSettings.touchTimeout);

		// Block events from touch
		if ((msg->message == WM_MOUSEMOVE && gStatus.gestureId != GID_PAN) ||
			msg->message == WM_NCHITTEST || msg->message == WM_NCMOUSEMOVE ||
			msg->message == WM_NCLBUTTONDOWN ||
			msg->message == WM_SETCURSOR || msg->message == WM_MOUSEWHEEL ||
			msg->message == WM_LBUTTONUP || msg->message == WM_LBUTTONDOWN ||
			msg->message == WM_RBUTTONUP || msg->message == WM_RBUTTONDOWN ||
			msg->message == WM_LBUTTONDBLCLK || msg->message == WM_RBUTTONDBLCLK ||
			msg->message == WM_KEYDOWN || msg->message == WM_KEYUP ||
			msg->message == WM_VSCROLL || msg->message == WM_HSCROLL) {
			if ((!bEnableTouch ||
					gSettings.lockTouch ||
					gStatus.gestureId != GID_BEGIN ||
					IsPainterWindow(msg->hwnd)) &&
				(GetMessageExtraInfo() & EVENTF_FROMTOUCH) == EVENTF_FROMTOUCH)
				msg->message = WM_USER + msg->message;
		}

		// Handle extra key down or long press
		if ((msg->message == WM_KEYDOWN && msg->wParam == gSettings.vkCode) &&
			gStatus.vkDownTick == 0) {
			gStatus.vkDownTick = tick;
			gStatus.vkMsgSent = FALSE;
		}
		BOOL bVkTimeout = (tick - gStatus.vkDownTick > gSettings.vkTimeout);
		if (((msg->message == WM_KEYUP && msg->wParam == gSettings.vkCode) || 
				(gStatus.vkDownTick > 0 && bVkTimeout)) &&
			!gStatus.vkMsgSent) {
			PostMessage(gSettings.nofityWnd, WM_USER + WM_COMMAND, bVkTimeout,
				gStatus.penHoverPos.x + gStatus.penHoverPos.y * 0x10000);
			gStatus.vkMsgSent = TRUE;
		}
		if (msg->message == WM_KEYUP && msg->wParam == gSettings.vkCode) {
			gStatus.vkDownTick = 0;
		}

		// The following messages (WM_USER + WM_GESTURE + 0/1/2) are posted from g_pManipulationEventSink
		// ON PROCESSING GESTURE
		if (msg->message == WM_USER + WM_GESTURE) {
			SHORT x = LOWORD(msg->lParam) - 0x8000, y = HIWORD(msg->lParam) - 0x8000,
				s = LOWORD(msg->wParam) - 0x8000, r = HIWORD(msg->wParam) - 0x8000;

			if (bEnableTouch) {
				// two finger gesture (zoom, rotate)
				if (gStatus.fingerCount == 2 && tick - gStatus.fingerTick[1] > 100 &&
					tick - gStatus.fingerTick[2] > 800 && tick - gStatus.fingerTick[3] > 800) {
					if ((r > gSettings.rotateTriMin && r < gSettings.rotateTriMax) &&
						(s < gSettings.zoomTriMin || s > gSettings.zoomTriMax))
						ChangeGesture(GID_ZOOM, s);
					else if ((r < gSettings.rotateTriMin || r > gSettings.rotateTriMax) &&
						(s > gSettings.zoomTriMin && s < gSettings.zoomTriMax))
						ChangeGesture(GID_ROTATE, r);
				}
				if ((gStatus.gestureId == GID_ZOOM || gStatus.gestureId == GID_ROTATE) &&
					(gStatus.fingerCount != 2 || (x*x + y*y) > 80*80))
					ChangeGesture(GID_BEGIN, 0);

				// one finger gesture (pan)
				if (gStatus.fingerCount == 1 && tick - gStatus.fingerTick[0] > 50 &&
					tick - gStatus.fingerTick[1] > 800 && tick - gStatus.fingerTick[2] > 800 &&
					gStatus.gestureId != GID_PAN) {
					POINT pt; GetCursorPos(&pt);
					if (IsPainterWindow(WindowFromPoint(pt))) {
						ChangeGesture(GID_PAN, 0);
					}
				}
				if (gStatus.fingerCount != 1 && gStatus.gestureId == GID_PAN)
					ChangeGesture(GID_BEGIN, 0);

				if (tick - gStatus.fingerTick[2] < gSettings.palmTimeout)
					ChangeGesture(GID_BEGIN, 0);
				else
					KeepGesture(x, y, s, r);

			}
			else {
				// Give up last gesture
				ChangeGesture(GID_BEGIN, 0);
			}
		}
		// TOUCHDOWN or TOUCHUP
		if (msg->message == WM_USER + WM_GESTURE + 1 || msg->message == WM_USER + WM_GESTURE + 2) {
			if (msg->wParam == 0 && msg->message == WM_USER + WM_GESTURE + 2)
				ChangeGesture(GID_BEGIN, 0);
			WORD x = LOWORD(msg->lParam), y = HIWORD(msg->lParam);
			if (gStatus.fingerCount > 0 && gStatus.fingerCount <= MAX_STATUS_FINGERS) {
				gStatus.fingerPos[gStatus.fingerCount - 1].x = x;
				gStatus.fingerPos[gStatus.fingerCount - 1].y = y;
				gStatus.fingerTick[gStatus.fingerCount - 1] = tick;
			}
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		DWORD tick = GetTickCount();
		CWPRETSTRUCT *cs = (CWPRETSTRUCT *)lParam;

		// send touch events to g_pIManipProc, and it will post (WM_USER + WM_GESTURE) message back when processing
		if (cs->message == WM_TOUCH) {
			if (g_pIManipProc == NULL || g_pManipulationEventSink == NULL) {
				CoInitialize(0);
				if (g_pIManipProc == NULL)
					CoCreateInstance(CLSID_ManipulationProcessor, NULL, CLSCTX_INPROC_SERVER,
						IID_IUnknown, (VOID**)(&g_pIManipProc));
				if (g_pIManipProc != NULL)
					g_pManipulationEventSink = new CManipulationEventSink(g_pIManipProc, gStatus.threadId);
			}
			if (g_pIManipProc != NULL && g_pManipulationEventSink != NULL) {
				HTOUCHINPUT hTi = (HTOUCHINPUT)cs->lParam;
				UINT cTi = LOWORD(cs->wParam);
				TOUCHINPUT *pTi = new TOUCHINPUT[cTi];
				if (GetTouchInputInfo(hTi, cTi, pTi, sizeof(TOUCHINPUT))) {
					for (UINT i = 0; i < cTi; i ++) {
						if (pTi[i].dwFlags & TOUCHEVENTF_DOWN) {
							gStatus.fingerCount = cTi;
							g_pIManipProc->ProcessDown(pTi[i].dwID, static_cast<FLOAT>(pTi[i].x), static_cast<FLOAT>(pTi[i].y));
							PostThreadMessage(gStatus.threadId, WM_USER + WM_GESTURE + 1, gStatus.fingerCount, MAKELONG(pTi[i].x/100, pTi[i].y/100));
						}
						if (pTi[i].dwFlags & TOUCHEVENTF_UP) {
							gStatus.fingerCount --;
							g_pIManipProc->ProcessUp(pTi[i].dwID, static_cast<FLOAT>(pTi[i].x), static_cast<FLOAT>(pTi[i].y));
							PostThreadMessage(gStatus.threadId, WM_USER + WM_GESTURE + 2, gStatus.fingerCount, MAKELONG(pTi[i].x/100, pTi[i].y/100));
						}
						if (pTi[i].dwFlags & TOUCHEVENTF_MOVE)
							g_pIManipProc->ProcessMove(pTi[i].dwID, static_cast<FLOAT>(pTi[i].x), static_cast<FLOAT>(pTi[i].y));
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
		if (cs->message == WM_USER + WM_QUIT) {
			RemoveProp(cs->hwnd, MICROSOFT_TABLETPENSERVICE_PROPERTY);
			UnregisterTouchWindow(cs->hwnd);
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

BOOL CALLBACK SendQuitMsgProc(HWND hWnd, LPARAM lParam) {
	SendMessage(hWnd, WM_USER + WM_QUIT, WM_QUIT, 0);
	EnumChildWindows(hWnd, SendQuitMsgProc, NULL);
	return TRUE;
}