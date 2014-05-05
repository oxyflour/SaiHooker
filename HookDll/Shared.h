#define MAX_SETTING_STEPS 64
#define MAX_VECTOR_LENGTH 64
#define MAX_STATUS_FINGERS 5

#define SAI_WINDOW_CLASS TEXT("sfl_window_class")
#define SAI_MENUBAR_CLASS TEXT("sfl_menubar_class")
#define SAI_PROP_WININFO TEXT("_SFLWININFO_")

#define WT_PACKET (WM_USER + 0x7FF0)
#define WM_PEN_HOVER_UNKNOWN 0x0ff2 // obtained from spyxx. not sure what's it

#define WM_USER_DEBUG (WM_USER + WM_APP)
#define WM_USER_QUIT (WM_USER + WM_QUIT)
#define WM_USER_GESTURE (WM_USER + WM_COMMAND + 1)
#define WM_USER_TOUCH (WM_USER + WM_COMMAND + 2)
#define WM_USER_GET_PEN (WM_USER + WM_COMMAND + 3)

#define WM_GESTURE_PROC (WM_USER + WM_GESTURE)
#define WM_GESTURE_DOWN (WM_USER + WM_GESTURE + 1)
#define WM_GESTURE_UP (WM_USER + WM_GESTURE + 2)

#define TIMEOUT_TOUCH_ENABLE_AFTER_PEN_HOVER 500
#define TIMEOUT_MOUSE_GESTURE_CLICK_INTERVAL 500
#define TIMEOUT_MOUSE_GESTURE_TAP_INTERVAL 200

#define DISTANCE_PAN_TRIGGER 10
#define DISTANCE_ZOOM_CANCEL 80
#define DISTANCE_MOUSE_GESTURE_BEGIN 35

#define MOUSE_GESTURE_REDUCE_COUNT 8
#define MOUSE_GESTURE_REDUCE_SLOPE 2.0
#define MOUSE_GESTURE_REDUCE_DISTANCE 0.20
#define MOUSE_GESTURE_REDUCE_RADIUS 0.55

#define SQUA(x) ((x)*(x))
#define SQUA_SUM(x, y) (SQUA(x)+ SQUA(y))
#define SQRT_SUM(x, y) (sqrt((double)SQUA_SUM(x, y)));

#define PostNotify(msg, wp, lp) PostThreadMessage(gStatus.notifyThread, (msg), (WPARAM)(wp), (LPARAM)(lp));

struct WINDOW_LIST {
	DWORD size;
	HWND list[64];
};

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
	DWORD penSize;
	char *penName;

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

struct SAI_WINDOWS {
	HWND main;
	HWND menu;

	HWND top;
	HWND zoom;
	HWND rotate;

	HWND nav;
	HWND layers;
	HWND color;
	HWND tools;
	HWND canvas;
};

extern SETTINGS gSettings;

extern STATUS gStatus;

extern SAI_WINDOWS gSaiWnds;

int ListIndex(EVENT_TRIGGER *pl, double val);
void GetChildWindowList(HWND hParent, WINDOW_LIST *pls);

void SimulateShortcut(SHORTCUT_KEY *pk, BOOL down);
void SimulateKey(WORD vk, DWORD flags);
void SimulateMouse(LONG dx, LONG dy, DWORD data, DWORD flags);

HWND GetSaiPenWindow();
BOOL IsSaiCanvasWindow(HWND hWnd);
int CheckSaiWindowList(SAI_WINDOWS *psw);
void GetSaiWindowAll(SAI_WINDOWS *psw);