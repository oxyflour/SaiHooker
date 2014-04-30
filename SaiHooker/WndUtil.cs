using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.Runtime.InteropServices;
using System.Drawing;

namespace SaiHooker
{
    [Guid("4661BF20-A332-4ADA-9F20-1D6EAB5DADA3")]
    public class WndUtil
    {
        const int GWL_STYLE = -16;
        const int GWL_EXSTYLE = -20;

        const int WS_CHILD = 0x40000000;
        const int WS_CLIPCHILDREN = 0x02000000;
//        const uint WS_POPUP = 0x80000000;

        const int WS_EX_TOPMOST = 0x00000008;
        const int WS_EX_NOACTIVATE = 0x08000000;
        const int WS_EX_TOOLWINDOW = 0x00000080;
        const int WS_EX_TRANSPARENT = 0x00000020;
        const int WS_EX_LAYERED = 0x00080000;

        const int HWND_TOPMOST = -1;

        const uint SWP_NOMOVE = 0x0002;
        const uint SWP_NOSIZE = 0x0001;

        const uint MF_STRING = 0x0000;
        const uint MF_POPUP = 0x0010;
        const uint MF_SEPARATOR = 0x0800;

        const uint TPM_CENTERALIGN = 0x0004;
        const uint TPM_VCENTERALIGN = 0x0010;
        const uint TPM_RETURNCMD = 0x0100;

        public void SetupToolWindow(string wndTitle)
        {
            m_hWnd = FindWindowEx(IntPtr.Zero, IntPtr.Zero, "HTML Application Host Window Class", wndTitle);
//            SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_CLIPCHILDREN | GetWindowLong(m_hWnd, GWL_STYLE));
            SetWindowLong(m_hWnd, GWL_EXSTYLE, WS_EX_NOACTIVATE | GetWindowLong(m_hWnd, GWL_EXSTYLE));
            // can not set WS_EX_TOPMOST directly
            SetWindowPos(m_hWnd, (IntPtr)HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }

        public void ActiveSaiWindow()
        {
            IntPtr hWnd = GetSaiWindow();
            if (hWnd != IntPtr.Zero)
                SetForegroundWindow(hWnd);
        }

        public void TextOut(string text, int x, int y)
        {
            IntPtr hdc = GetDC(IntPtr.Zero);
            /*
            Graphics gc = Graphics.FromHdc(hdc);
            Font f = new Font(FontFamily.GenericSansSerif, 15);
            SizeF sz = gc.MeasureString(text, f);
            RectangleF rt = new RectangleF(x, y, sz.Width, sz.Height);
            gc.FillRectangle(new SolidBrush(Color.White), rt);
            gc.DrawString(text, f, new SolidBrush(Color.Black), rt);
            */
            TextOut(hdc, x, y, text, text.Length);
            ReleaseDC(IntPtr.Zero, hdc);
        }

        public int PopupMenu(string menu, int x, int y)
        {
            IntPtr hMenu = CreateMenu();
            IntPtr hPopup = CreatePopupMenu();
            XmlDocument doc = new XmlDocument();
            doc.LoadXml(menu);

            ParseMenu(hPopup, doc.DocumentElement);
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (uint)(long)hPopup, "popup");

            int ret = TrackPopupMenu(hPopup, TPM_CENTERALIGN | TPM_VCENTERALIGN | TPM_RETURNCMD, x, y, 0, m_hWnd, 0);
            DestroyMenu(hMenu);

            return ret;
        }

        void ParseMenu(IntPtr hParent, XmlNode node)
        {
            foreach (XmlNode n in node.ChildNodes)
            {
                String name = n.Name.ToLower();
                if (name == "menu") {
                    IntPtr hSub = CreatePopupMenu();
                    ParseMenu(hSub, n);
                    AppendMenu(hParent, MF_STRING | MF_POPUP, (uint)(long)hSub,
                        n.Attributes["text"].Value.ToString());
                }
                else if (name == "item")
                {
                    if (n.Attributes["hidden"] == null)
                        AppendMenu(hParent, MF_STRING, uint.Parse(n.Attributes["cid"].Value.ToString()),
                            n.Attributes["text"].Value.ToString());
                }
                else if (name == "sep")
                {
                    AppendMenu(hParent, MF_SEPARATOR, 0, null);
                }
            }
        }

        IntPtr m_hWnd;

        [DllImport(Hooker.DLL_NAME, CharSet = CharSet.Auto)]
        private static extern IntPtr GetSaiWindow();

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr FindWindowEx(IntPtr hParent, IntPtr hPrev, String lpClassName, String lpWndName);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int GetWindowTextLength(IntPtr hWnd);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int SetWindowLong(IntPtr hWnd, int nIndex, int dwNewLong);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int GetWindowLong(IntPtr hWnd, int nIndex);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int SetWindowPos(IntPtr hWnd, IntPtr hAfter, int x, int y, int cx, int cy, uint flags);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr SetActiveWindow(IntPtr hWnd);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr SetParent(IntPtr hWndChild, IntPtr hWndParent);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int SetForegroundWindow(IntPtr hWnd);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr CreateMenu();

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr CreatePopupMenu();

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int AppendMenu(IntPtr hMenu, uint uFlags, uint uIDNewItem, string lpNewItem);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int DestroyMenu(IntPtr hMenu);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int TrackPopupMenu(IntPtr hMenu, uint uFlags, int x, int y, int nReserved, IntPtr hWnd, int prcRect);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr GetDC(IntPtr hWnd);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int ReleaseDC(IntPtr hWnd, IntPtr hdc);

        [DllImport("gdi32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int TextOut(IntPtr hdc, int x, int y, String text, int len);
    }
}
