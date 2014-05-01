#define MAX_SETTING_STEPS 64
#define MAX_VECTOR_LENGTH 64
#define MAX_STATUS_FINGERS 5
#define MAX_BUTTON_COUNT 64
#define MAX_BUTTON_TEXT 128

#define SAI_WINDOW_CLASS TEXT("sfl_window_class")
#define SAI_MENUBAR_CLASS TEXT("sfl_menubar_class")

#define WT_PACKET (WM_USER + 0x7FF0)
#define WM_PEN_HOVER_UNKNOWN 0x0ff2 // obtained from spyxx. not sure what's it

#define WM_USER_DEBUG (WM_USER + WM_APP)
#define WM_USER_QUIT (WM_USER + WM_QUIT)
#define WM_USER_GESTURE (WM_USER + WM_COMMAND + 1)
#define WM_USER_TOUCH (WM_USER + WM_COMMAND + 2)
#define WM_GESTURE_PROC (WM_USER + WM_GESTURE)
#define WM_GESTURE_DOWN (WM_USER + WM_GESTURE + 1)
#define WM_GESTURE_UP (WM_USER + WM_GESTURE + 2)

#define TIMEOUT_TOUCH_ENABLE_AFTER_PEN_HOVER 500
#define TIMEOUT_GESTURE_ENABLE_AFTER_PALM 800
#define TIMEOUT_MOUSE_GESTURE_FINISH_DELAY 50

#define DISTANCE_PAN_TRIGGER 10
#define DISTANCE_ZOOM_CANCEL 80

#define SQUA(x) ((x)*(x))
#define SQUA_SUM(x, y) (SQUA(x)+ SQUA(y))
#define SQRT_SUM(x, y) (sqrt((double)SQUA_SUM(x, y)));

#define PostNotify(msg, wp, lp) PostThreadMessage(gStatus.notifyThread, (msg), (wp), (lp));

struct SHORTCUT_KEY {
	BOOL enabled;
	BOOL pressed;
	WORD vk;
	BOOL ctrl;
	BOOL shift;
	BOOL alt;
};

struct EVENT_TRIGGER {
	UINT msg;
	DWORD index;
	DWORD size;
	double list[MAX_SETTING_STEPS];
};

struct SETTINGS {
	DWORD lockTouch;

	DWORD painterLeaveTimeout;

	DWORD panVkCode;
	DWORD vkTimeout;
	DWORD fingerTapInteval;

	DWORD mgEnableTimeout;
	DWORD mgEnableDistance;
	DWORD mgPointCount;
	double mgSlope;
	double mgDistanceIn;
	double mgRadiusIn;

	SHORTCUT_KEY dragKey;
	EVENT_TRIGGER evtOffsetX;
	EVENT_TRIGGER evtOffsetY;
	EVENT_TRIGGER evtZoom;
	EVENT_TRIGGER evtRotate;
};

struct STATUS {
	// SAI thread id
	DWORD targetThread;
	// Hooker thread id
	DWORD notifyThread;

	// pen hover time & position
	DWORD penHoverTick;
	POINT penHoverPos;

	// touch gesture status
	DWORD tgState;
	DWORD tgFingers;
	HWND tgWindow;
	int tgScale;
	int tgRotate;
	DWORD tgDownTicks[MAX_STATUS_FINGERS];
	DWORD tgUpTicks[MAX_STATUS_FINGERS];

	// mouse gesture status
	BOOL isLeftDown;
	BOOL isRightDown;
	DWORD mgTick;
	DWORD mgState;
	POINT mgBeginPos;
	TCHAR mgVectorStr[MAX_VECTOR_LENGTH];
};

extern SETTINGS gSettings;

extern STATUS gStatus;

int ListIndex(EVENT_TRIGGER *pl, double val);
BOOL IsPainterWindow(HWND hWnd);
void SimulateShortcut(SHORTCUT_KEY *pk, BOOL down);
void SimulateKey(WORD vk, DWORD flags);
void SimulateMouse(LONG dx, LONG dy, DWORD data, DWORD flags);