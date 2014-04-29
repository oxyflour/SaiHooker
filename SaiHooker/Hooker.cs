using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace SaiHooker
{
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    public interface IHookEvent
    {
        [DispId(1)]
        void OnHookEvent(int msg, int wParam, int lParam);
        [DispId(3)]
        void OnMouseGesture(string vec, int key, int x, int y);
        [DispId(4)]
        void OnFingerTap(int n, int x, int y);
    }

    [Guid("8E1D1128-0685-4C1D-8475-916B2BDE241A")]
    [ComSourceInterfaces(typeof(IHookEvent))]
    public class Hooker
    {
        public const string DLL_NAME = "HookDll.dll";

        const int WH_GETMESSAGE = 3;
        const int WH_CALLWNDPROCRET = 12;
        const int HC_ACTION = 0;

        const uint WM_APP = 0x8000;
        const uint WM_USER = 0x0400;
        const uint WM_COMMAND = 0x0111;

        const uint WM_USER_DEBUG = WM_USER + WM_APP;
        const uint WM_USER_GESTURE = WM_USER + WM_COMMAND + 1;
        const uint WM_USER_FINGERTAP = WM_USER + WM_COMMAND + 2;

        [StructLayout(LayoutKind.Sequential)]
        struct POINT
        {
            public int x;
            public int y;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct MSG
        {
            public IntPtr hwnd;
            public uint message;
            public UIntPtr wParam;
            public IntPtr lParam;
            public uint time;
            public POINT pt;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct CWPRETSTRUCT
        {
            public IntPtr lResult;
            public IntPtr lParam;
            public UIntPtr wParam;
            public uint message;
            public IntPtr hwnd;
        }

        static Hooker s_this = null;

        public static unsafe int GetMsgProc(int nCode, UIntPtr wParam, IntPtr lParam)
        {
            if (nCode == HC_ACTION)
            {
//                MSG msg = (MSG)Marshal.PtrToStructure(lParam, typeof(MSG));
                MSG* msg = (MSG*)lParam;
                if (msg->message >= WM_USER_DEBUG && msg->message <= WM_USER_DEBUG + 16 && s_this.OnHookEvent != null)
                {
                    s_this.OnHookEvent((int)msg->message - (int)WM_USER_DEBUG, (int)(ulong)msg->wParam, (int)msg->lParam);
                }
                if (msg->message == WM_USER_GESTURE && s_this.OnMouseGesture != null)
                {
                    StringBuilder sz = new StringBuilder(64);
                    GetVectorStr(sz, sz.Capacity);
                    int x = (int)msg->lParam % 0x10000, y = (int)msg->lParam / 0x10000;
                    s_this.OnMouseGesture(sz.ToString(), (int)msg->wParam, x, y);
                }
                if (msg->message == WM_USER_FINGERTAP && s_this.OnFingerTap != null)
                {
                    int x = (int)msg->lParam % 0x10000, y = (int)msg->lParam / 0x10000;
                    s_this.OnFingerTap((int)msg->wParam, x, y);
                }
            }
            return CallNextHookEx(s_this.m_hHook, nCode, wParam, lParam);
        }
        private static HookProc hookProc = new HookProc(GetMsgProc);

        public Hooker()
        {
            if (s_this == null)
            {
                uint tid = GetCurrentThreadId();
                if ((uint)tid > 0)
                    m_hHook = SetWindowsHookEx(WH_GETMESSAGE, hookProc, IntPtr.Zero, tid);
                s_this = this;
            }
        }

        ~Hooker()
        {
            if (m_hHook != IntPtr.Zero)
            {
                UnhookWindowsHookEx(m_hHook);
                s_this = null;
            }
        }

        IntPtr m_hInst = IntPtr.Zero;
        IntPtr m_hHook = IntPtr.Zero;

        public delegate void HookEventHandle(int msg, int wParam, int lParam);
        public event HookEventHandle OnHookEvent;

        public delegate void MouseGestureHandle(string vec, int key, int x, int y);
        public event MouseGestureHandle OnMouseGesture;

        public delegate void FingerTapHandle(int n, int x, int y);
        public event FingerTapHandle OnFingerTap;

        public uint Hook()
        {
            if (m_hInst == IntPtr.Zero)
                m_hInst = LoadLibrary(DLL_NAME);
            return m_hInst == IntPtr.Zero ? (uint)Marshal.GetLastWin32Error() : SetSaiHook(m_hInst);
        }

        public void UnHook()
        {
            if (m_hInst != IntPtr.Zero)
            {
                UnsetSaiHook();
                FreeLibrary(m_hInst);
            }
            m_hInst = IntPtr.Zero;
        }

        public int DisableTouch(int l)
        {
            return LockTouch(l);
        }

        public int SetPanningKey(int k)
        {
            return SetPanningVk(k);
        }

        public void SimulateKey(int vkCode, bool keyDown)
        {
            SimulateKeyEvent(vkCode, keyDown);
        }

        public void SimulateMouse(int x, int y, bool keyDown)
        {
            SimulateMouseEvent(x, y, keyDown);
        }

        public void SimulateDragWith(int vk, bool ctrl, bool shift, bool alt)
        {
            SimulateDragWithKey(vk, ctrl, shift, alt);
        }

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern uint SetSaiHook(IntPtr hInst);

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern void UnsetSaiHook();

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern int LockTouch(int l);

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern int SetPanningVk(int l);

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern int GetVectorStr(StringBuilder szBuf, int size);

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern void SimulateKeyEvent(int vkCode, bool keyDown);

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern void SimulateMouseEvent(int x, int y, bool keyDown);

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern void SimulateDragWithKey(int vk, bool ctrl, bool shift, bool alt);

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr LoadLibrary(String dllToLoad);

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int FreeLibrary(IntPtr hModule);

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern uint GetCurrentThreadId();

        public delegate int HookProc(int nCode, UIntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr SetWindowsHookEx(int idHook, HookProc lpfn, IntPtr hMod, uint dwThreadId);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int UnhookWindowsHookEx(IntPtr hHook);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int CallNextHookEx(IntPtr hHook, int nCode, UIntPtr wParam, IntPtr lParam);
    }
}
