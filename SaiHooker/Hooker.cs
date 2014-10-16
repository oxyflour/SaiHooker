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
        void OnTouchGesture(int n, int k, int x, int y);
    }

    [Guid("8E1D1128-0685-4C1D-8475-916B2BDE241A")]
    [ComSourceInterfaces(typeof(IHookEvent))]
    public class Hooker
    {
        public const string DLL_NAME = "HookDll.dll";

        const int WH_GETMESSAGE = 3;
        const int WH_CALLWNDPROC = 4;
        const int WH_CALLWNDPROCRET = 12;
        const int HC_ACTION = 0;

        const uint WM_NCLBUTTONDOWN = 0x00A1;
        const uint WM_APP = 0x8000;
        const uint WM_USER = 0x0400;
        const uint WM_COMMAND = 0x0111;

        const uint WM_ACTIVATE = 0x0006;
        const uint WM_ACTIVATEAPP = 0x001C;
        const uint WM_NCACTIVATE = 0x0086;

        const uint WM_USER_DEBUG = WM_USER + WM_APP;
        const uint WM_USER_GESTURE = WM_USER + WM_COMMAND + 1;
        const uint WM_USER_TOUCH = WM_USER + WM_COMMAND + 2;

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
        struct CWPSTRUCT
        {
            public IntPtr lParam;
            public UIntPtr wParam;
            public uint message;
            public IntPtr hwnd;
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
                MSG msg = (MSG)Marshal.PtrToStructure(lParam, typeof(MSG));
                if (msg.message >= WM_USER_DEBUG && msg.message <= WM_USER_DEBUG + 64 && s_this.OnHookEvent != null)
                {
                    s_this.OnHookEvent((int)msg.message - (int)WM_USER_DEBUG, (int)(ulong)msg.wParam, (int)msg.lParam);
                }
                if (msg.message == WM_USER_GESTURE && s_this.OnMouseGesture != null)
                {
                    StringBuilder sz = new StringBuilder(64);
                    SaiStatus("gesture-vector", null, sz, sz.Capacity);
                    int x = (int)msg.lParam % 0x10000, y = (int)msg.lParam / 0x10000;
                    s_this.OnMouseGesture(sz.ToString(), (int)msg.wParam, x, y);
                }
                if (msg.message == WM_USER_TOUCH && s_this.OnTouchGesture != null)
                {
                    int x = (int)msg.lParam % 0x10000, y = (int)msg.lParam / 0x10000;
                    int n = (int)msg.wParam % 0x10000, k = (int)msg.wParam / 0x10000;
                    s_this.OnTouchGesture(n, k, x, y);
                }
                // active the window
                if (msg.message == WM_NCLBUTTONDOWN)
                {
                    WndUtil.SetForegroundWindow(msg.hwnd);
                }
            }
            return CallNextHookEx(s_this.m_hkMsg, nCode, wParam, lParam);
        }
        private static HookProc s_pMsg = new HookProc(GetMsgProc);

        public static unsafe int WndProc(int nCode, UIntPtr wParam, IntPtr lParam)
        {
            if (nCode == HC_ACTION)
            {
                CWPSTRUCT cs = (CWPSTRUCT)Marshal.PtrToStructure(lParam, typeof(CWPSTRUCT));
            }
            return CallNextHookEx(s_this.m_hkProc, nCode, wParam, lParam);
        }
        private static HookProc s_pProc = new HookProc(WndProc);

        public Hooker()
        {
            if (s_this == null)
            {
                uint tid = GetCurrentThreadId();
                if ((uint)tid > 0)
                {
                    m_hkMsg = SetWindowsHookEx(WH_GETMESSAGE, s_pMsg, IntPtr.Zero, tid);
                    m_hkProc = SetWindowsHookEx(WH_CALLWNDPROC, s_pProc, IntPtr.Zero, tid);
                }
                s_this = this;
            }
        }

        ~Hooker()
        {
            if (m_hkMsg != IntPtr.Zero || m_hkProc != IntPtr.Zero)
            {
                UnhookWindowsHookEx(m_hkMsg);
                UnhookWindowsHookEx(m_hkProc);
                s_this = null;
            }
        }

        IntPtr m_hInst = IntPtr.Zero;
        IntPtr m_hkMsg = IntPtr.Zero;
        IntPtr m_hkProc = IntPtr.Zero;

        public delegate void HookEventHandle(int msg, int wParam, int lParam);
        public event HookEventHandle OnHookEvent;

        public delegate void MouseGestureHandle(string vec, int key, int x, int y);
        public event MouseGestureHandle OnMouseGesture;

        public delegate void FingerTapHandle(int n, int k, int x, int y);
        public event FingerTapHandle OnTouchGesture;

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

        public string SetSaiStatus(string key, string val)
        {
            StringBuilder ret = new StringBuilder(64);
            SaiStatus(key, val, ret, ret.Capacity);
            return ret.ToString();
        }
        public string GetSaiStatus(string key)
        {
            return SetSaiStatus(key, null);
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

        public void RegisterNotify(int msg, string evt, string steps)
        {
            RegisterEventNotify(msg, evt, steps);
        }

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern uint SetSaiHook(IntPtr hInst);

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern void UnsetSaiHook();

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern int SaiStatus(string key, string val, StringBuilder ret, int size);

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern void SimulateKeyEvent(int vkCode, bool keyDown);

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern void SimulateMouseEvent(int x, int y, bool keyDown);

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern void SimulateDragWithKey(int vk, bool ctrl, bool shift, bool alt);

        [DllImport(DLL_NAME, CharSet = CharSet.Auto)]
        private static extern void RegisterEventNotify(int msg, string evt, string steps);

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
