// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� HOOKDLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// HOOKDLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef HOOKDLL_EXPORTS
#define HOOKDLL_API __declspec(dllexport)
#else
#define HOOKDLL_API __declspec(dllimport)
#endif

/*
// �����Ǵ� HookDll.dll ������
class HOOKDLL_API CHookDll {
public:
	CHookDll(void);
	// TODO: �ڴ�������ķ�����
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
