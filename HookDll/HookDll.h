// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 HOOKDLL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// HOOKDLL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef HOOKDLL_EXPORTS
#define HOOKDLL_API __declspec(dllexport)
#else
#define HOOKDLL_API __declspec(dllimport)
#endif

/*
// 此类是从 HookDll.dll 导出的
class HOOKDLL_API CHookDll {
public:
	CHookDll(void);
	// TODO: 在此添加您的方法。
};

extern HOOKDLL_API int nHookDll;

HOOKDLL_API int fnHookDll(void);
*/

extern "C" {

	HOOKDLL_API HWND _stdcall GetSaiWindow();

	HOOKDLL_API int _stdcall GetSaiStatus(TCHAR *key);

	HOOKDLL_API HWND _stdcall SetNotifyWindow(HWND hWnd);

	HOOKDLL_API DWORD _stdcall SetSaiHook(HINSTANCE hInst);

	HOOKDLL_API void _stdcall UnsetSaiHook();

	HOOKDLL_API int _stdcall LockTouch(int lock);

	HOOKDLL_API int _stdcall GetmgVectorStr(TCHAR* szBuf, int size);

	HOOKDLL_API void _stdcall SimulateKeyEvent(int vk, bool down);

	HOOKDLL_API void _stdcall SimulateMouseEvent(int x, int y, bool down);

	HOOKDLL_API void _stdcall SimulateDragWithKey(int vk, bool ctrl, bool shift, bool alt);

	HOOKDLL_API void _stdcall RegisterEventNotify(int msg, TCHAR *evt, TCHAR *steps);
}
