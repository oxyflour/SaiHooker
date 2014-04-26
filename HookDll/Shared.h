#define MAX_SETTING_STEPS 64
#define MAX_VECTOR_LENGTH 64
#define MAX_STATUS_FINGERS 5

#define SAI_WINDOW_CLASS TEXT("sfl_window_class")
#define SAI_MENUBAR_CLASS TEXT("sfl_menubar_class")

#define WM_USER_DEBUG (WM_USER + WM_APP)
#define WM_USER_QUIT (WM_USER + WM_QUIT)
#define WM_USER_VIRTUALKEY (WM_USER + WM_COMMAND)
#define WM_USER_FINGERTAP (WM_USER + WM_COMMAND + 2)
#define WM_USER_GESTURE (WM_USER + WM_COMMAND + 1)
#define WM_GESTURE_PROC (WM_USER + WM_GESTURE)
#define WM_GESTURE_DOWN (WM_USER + WM_GESTURE + 1)
#define WM_GESTURE_UP (WM_USER + WM_GESTURE + 2)

#define SQUA(x) ((x)*(x))
#define SQUA_SUM(x, y) (SQUA(x)+ SQUA(y))
#define SQRT_SUM(x, y) (sqrt((double)SQUA_SUM(x, y)));

struct SETTINGS {
	DWORD lockTouch;

	// window that receives message
	HWND nofityWnd;

	DWORD touchEnableTimeout;
	DWORD guestureEnableTimeout;
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

	DWORD panTriggerDistance;
	DWORD guestureCancelDistance;
	double zoomTriMin;
	double zoomTriMax;
	double zoomArr[MAX_SETTING_STEPS];
	double rotateTriMin;
	double rotateTriMax;
	double rotateArr[MAX_SETTING_STEPS];
};

struct STATUS {
	// SAI thread id
	DWORD threadId;

	// pen hover time & position
	DWORD penHoverTick;
	POINT penHoverPos;

	// gesture id (GID_XXX) and how many fingers is on
	DWORD gestureId;
	DWORD fingerCount;
//	POINT fingerPos[MAX_STATUS_FINGERS];
//	DWORD fingerTick[MAX_STATUS_FINGERS];
	DWORD fingerDownTick[MAX_STATUS_FINGERS];
	DWORD fingerUpTick[MAX_STATUS_FINGERS];

	BOOL isCtrlDown;
	BOOL isAltDown;
	DWORD vkDownTick;
	DWORD vkStateId;
	POINT vkPenPos;

	DWORD painterLeaveTick;

	DWORD panVkState;
	int zoomIndex;
	int rotateIndex;

	TCHAR vectorStr[MAX_VECTOR_LENGTH];
};

extern SETTINGS gSettings;

extern STATUS gStatus;

int FindInArray(double *arr, int size, double value);
BOOL IsPainterWindow(HWND hWnd);
HWND GetLogWindow();
void LogText(TCHAR *szBuf, ...);
void SimulateKey(WORD vk, DWORD flags);
void SimulateMouse(LONG dx, LONG dy, DWORD data, DWORD flags);