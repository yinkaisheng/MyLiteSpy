// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "targetver.h"
//#include <afxpriv.h>
#include <windows.h>
#include <CommCtrl.h>
#include <commdlg.h>
#include <Richedit.h>
#include <Ime.h>
#include <shellapi.h>
#include <dde.h>
#include <stdio.h>
#include <map>
#include "myhook.h"

//Initialized Data to be shared with all instance of the dll
#pragma data_seg("Shared")
HWND g_hNotifyWnd = NULL;
HWND g_hCaptureWnd = NULL;
HINSTANCE g_hInstance=NULL;
HHOOK g_hCBTHook = NULL;
HHOOK g_hCallWndProcHook = NULL;
HHOOK g_hCallWndProcRetHook = NULL;
HHOOK g_hGetMessageHook = NULL;
#pragma data_seg()
// Initialised data End of data share
#pragma comment(linker,"/section:Shared,RWS")

#ifndef CB_MULTIPLEADDSTRING
#define CB_MULTIPLEADDSTRING        0x0163
#endif

#ifndef LB_MULTIPLEADDSTRING
#define LB_MULTIPLEADDSTRING    0x01B1
#endif

#ifndef EM_NOSETFOCUS
#define EM_NOSETFOCUS       (ECM_FIRST + 7)
#endif

#ifndef EM_TAKEFOCUS
#define EM_TAKEFOCUS        (ECM_FIRST + 8)
#endif

#ifndef PGM_SETSCROLLINFO
#define PGM_SETSCROLLINFO      (PGM_FIRST + 13)
#endif

#ifndef TB_HASACCELERATOR
#define TB_HASACCELERATOR       (WM_USER + 95)  // wParam == char, lParam = &iCount
#endif

#ifndef TB_SETBOUNDINGSIZE
#define TB_SETBOUNDINGSIZE      (WM_USER + 93)
#endif

#ifndef TVM_SETBORDER
#define TVM_SETBORDER         (TV_FIRST + 35)
#endif

#ifndef TVM_SETHOT
#define TVM_SETHOT                 (TV_FIRST + 58)
#endif

//copy from afxpriv.h
/////////////////////////////////////////////////////////////////////////////
// Internal AFX Windows messages (see Technical note TN024 for more details)
// (0x0360 - 0x037F are reserved for MFC)

#define WM_QUERYAFXWNDPROC  0x0360  // lResult = 1 if processed by AfxWndProc
#define WM_SIZEPARENT       0x0361  // lParam = &AFX_SIZEPARENTPARAMS
#define WM_SETMESSAGESTRING 0x0362  // wParam = nIDS (or 0),
// lParam = lpszOther (or NULL)
#define WM_IDLEUPDATECMDUI  0x0363  // wParam == bDisableIfNoHandler
#define WM_INITIALUPDATE    0x0364  // (params unused) - sent to children
#define WM_COMMANDHELP      0x0365  // lResult = TRUE/FALSE,
// lParam = dwContext
#define WM_HELPHITTEST      0x0366  // lResult = dwContext,
// lParam = MAKELONG(x,y)
#define WM_EXITHELPMODE     0x0367  // (params unused)
#define WM_RECALCPARENT     0x0368  // force RecalcLayout on frame window
//  (only for inplace frame windows)
#define WM_SIZECHILD        0x0369  // special notify from COleResizeBar
// wParam = ID of child window
// lParam = lpRectNew (new position/size)
#define WM_KICKIDLE         0x036A  // (params unused) causes idles to kick in
#define WM_QUERYCENTERWND   0x036B  // lParam = HWND to use as centering parent
#define WM_DISABLEMODAL     0x036C  // lResult = 0, disable during modal state
// lResult = 1, don't disable
#define WM_FLOATSTATUS      0x036D  // wParam combination of FS_* flags below

// WM_ACTIVATETOPLEVEL is like WM_ACTIVATEAPP but works with hierarchies
//   of mixed processes (as is the case with OLE in-place activation)
#define WM_ACTIVATETOPLEVEL 0x036E  // wParam = nState (like WM_ACTIVATE)
// lParam = pointer to HWND[2]
//  lParam[0] = hWnd getting WM_ACTIVATE
//  lParam[1] = hWndOther

#define WM_RESERVED_036F	0x036F  // was WM_QUERY3DCONTROLS (now not used)

// Note: Messages 0x0370, 0x0371, and 0x372 were incorrectly used by
//  some versions of Windows.  To remain compatible, MFC does not
//  use messages in that range.
#define WM_RESERVED_0370    0x0370
#define WM_RESERVED_0371    0x0371
#define WM_RESERVED_0372    0x0372

// WM_SOCKET_NOTIFY and WM_SOCKET_DEAD are used internally by MFC's
// Windows sockets implementation.  For more information, see sockcore.cpp
#define WM_SOCKET_NOTIFY    0x0373
#define WM_SOCKET_DEAD      0x0374

// same as WM_SETMESSAGESTRING except not popped if IsTracking()
#define WM_POPMESSAGESTRING 0x0375

// WM_HELPPROMPTADDR is used internally to get the address of 
//	m_dwPromptContext from the associated frame window. This is used
//	during message boxes to setup for F1 help while that msg box is
//	displayed. lResult is the address of m_dwPromptContext.
#define WM_HELPPROMPTADDR	0x0376

// Constants used in DLGINIT resources for OLE control containers
// NOTE: These are NOT real Windows messages they are simply tags
// used in the control resource and are never used as 'messages'
#define WM_OCC_LOADFROMSTREAM           0x0376
#define WM_OCC_LOADFROMSTORAGE          0x0377
#define WM_OCC_INITNEW                  0x0378
#define WM_OCC_LOADFROMSTREAM_EX        0x037A
#define WM_OCC_LOADFROMSTORAGE_EX       0x037B

// Constant used in DLGINIT resources for MFC controls
#define WM_MFC_INITCTRL                 0x037C

// Marker used while rearranging the message queue
#define WM_QUEUE_SENTINEL   0x0379

// Note: Messages 0x037D - 0x37E reserved for future MFC use.
#define WM_RESERVED_037D    0x037D
#define WM_RESERVED_037E    0x037E

// WM_FORWARDMSG - used by ATL to forward a message to another window for processing
//	WPARAM - DWORD dwUserData - defined by user
//	LPARAM - LPMSG pMsg - a pointer to the MSG structure
//	return value - 0 if the message was not processed, nonzero if it was
#define WM_FORWARDMSG		0x037F


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        {
            g_hInstance = hModule;
            break;
        }
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


#ifdef __cplusplus
extern "C"
{
#endif

static LRESULT CALLBACK CBTProc(int nCode,WPARAM wParam,LPARAM lParam)
{
    if (nCode >= 0)
    {
        if (nCode == HCBT_ACTIVATE)  //Called when the application window is activated
        {
            ::PostMessage(g_hNotifyWnd, WM_NotifyActivate, wParam, NULL);
        }
        else if (nCode == HCBT_SETFOCUS)
        {
            ::PostMessage(g_hNotifyWnd, WM_NotifyFocus, wParam, NULL);
        }
        else if (nCode == HCBT_DESTROYWND) //Called when the application window is destroyed
        {

        }
    }
    return CallNextHookEx(g_hCBTHook, nCode, wParam, lParam);
}

DLL_EXPORT bool InstallCBTHook(HWND hNotifyWnd)
{
    g_hNotifyWnd = hNotifyWnd;

    if (!g_hCBTHook)
    {
        g_hCBTHook = SetWindowsHookEx(WH_CBT, (HOOKPROC)CBTProc, g_hInstance, 0);

        if (g_hCBTHook)
        {
            OutputDebugStringA("Hook CBT succeed\n");
            return true;
        }
        else
        {
            DWORD dwError = GetLastError();
            char szError[MAX_PATH];
            _snprintf_s(szError, MAX_PATH, "Hook CBT failed, error = %u\n", dwError);
            OutputDebugStringA(szError);
        }
    }

    return false;
}

DLL_EXPORT bool UninstallCBTHook()
{
    if (g_hCBTHook)
    {
        UnhookWindowsHookEx(g_hCBTHook);
        g_hCBTHook = NULL;
        OutputDebugStringA("Uninstall CBT Hook\n");
    }

    return true;
}

//note:
//CallWndProc will be executed in the process which myhook.dll injected, not the MySpy process
static LRESULT CALLBACK CallWndProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code == HC_ACTION) 
    {
        PCWPSTRUCT pMsg = (PCWPSTRUCT)lParam;
        if (pMsg->hwnd == g_hCaptureWnd)
        {
            ::PostMessage(g_hNotifyWnd, WM_NotifyCallWndProc, (WPARAM)pMsg->hwnd, pMsg->message);
            //char szBuf[MAX_PATH] = {0};
            //_snprintf_s(szBuf, MAX_PATH, "CallWndProc Handle: 0x%08X SendMsg: %s(%04X), wParam: %08X, lParam: %08X\n", 
            //    pMsg->hwnd, GetMsgStringA(pMsg->message), pMsg->message, (int)pMsg->wParam, (int)pMsg->lParam);
            //OutputDebugStringA(szBuf);
        }
    }

    return CallNextHookEx(g_hCallWndProcHook, code, wParam, lParam);
}

DLL_EXPORT bool InstallCallWndProcHook(HWND hNotifyWnd, HWND hCaptureWnd)
{
    g_hNotifyWnd = hNotifyWnd;
    g_hCaptureWnd = hCaptureWnd;

    if (!g_hCallWndProcHook)
    {
        DWORD dwThreadId = ::GetWindowThreadProcessId(g_hCaptureWnd, NULL);
        g_hCallWndProcHook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, g_hInstance, dwThreadId);

        if (g_hCallWndProcHook)
        {
            OutputDebugStringA("Hook CallWndProc succeed\n");
            return true;
        }
        else
        {
            DWORD dwError = GetLastError();
            char szError[MAX_PATH];
            _snprintf_s(szError, MAX_PATH, "Hook CallWndProc failed, error = %u\n", dwError);
            OutputDebugStringA(szError);
        }
    }

    return false;
}

DLL_EXPORT bool UninstallCallWndProcHook()
{
    if (g_hCallWndProcHook)
    {
        UnhookWindowsHookEx(g_hCallWndProcHook);
        g_hCallWndProcHook = NULL;
        OutputDebugStringA("Uninstall CallWndProc Hook\n");
    }

    return true;
}

//note:
//CallWndProc will be executed in the process which myhook.dll injected, not the MySpy process
static LRESULT CALLBACK CallWndProcRet(int code, WPARAM wParam, LPARAM lParam)
{
    if (code == HC_ACTION) 
    {
        PCWPRETSTRUCT pMsg = (PCWPRETSTRUCT)lParam;
        if (pMsg->hwnd == g_hCaptureWnd)
        {
            ::PostMessage(g_hNotifyWnd, WM_NotifyCallWndProcRet, (WPARAM)pMsg->hwnd, pMsg->message);
            if (WM_NCDESTROY == pMsg->message)
            {
                UninstallCallWndProcHook();
                UninstallCallWndProcRetHook();
                UninstallGetMessageHook();
            }
            //char szBuf[MAX_PATH] = {0};
            //_snprintf_s(szBuf, MAX_PATH, "CallWndProcRet Handle: 0x%08X SendMsg: %s(%04X), wParam: %08X, lParam: %08X\n", 
            //    pMsg->hwnd, GetMsgStringA(pMsg->message), pMsg->message, (int)pMsg->wParam, (int)pMsg->lParam);
            //OutputDebugStringA(szBuf);
        }
    }

    return CallNextHookEx(g_hCallWndProcRetHook, code, wParam, lParam);
}

DLL_EXPORT bool InstallCallWndProcRetHook(HWND hNotifyWnd, HWND hCaptureWnd)
{
    g_hNotifyWnd = hNotifyWnd;
    g_hCaptureWnd = hCaptureWnd;

    if (!g_hCallWndProcRetHook)
    {
        DWORD dwThreadId = ::GetWindowThreadProcessId(g_hCaptureWnd, NULL);
        g_hCallWndProcRetHook = SetWindowsHookEx(WH_CALLWNDPROCRET, (HOOKPROC)CallWndProcRet, g_hInstance, dwThreadId);

        if (g_hCallWndProcRetHook)
        {
            OutputDebugStringA("Hook CallWndProcRet succeed\n");
            return true;
        }
        else
        {
            DWORD dwError = GetLastError();
            char szError[MAX_PATH];
            _snprintf_s(szError, MAX_PATH, "Hook CallWndProcRet failed, error = %u\n", dwError);
            OutputDebugStringA(szError);
        }
    }

    return false;
}

DLL_EXPORT bool UninstallCallWndProcRetHook()
{
    if (g_hCallWndProcRetHook)
    {
        UnhookWindowsHookEx(g_hCallWndProcRetHook);
        g_hCallWndProcRetHook = NULL;
        OutputDebugStringA("Uninstall CallWndProcRet Hook\n");
    }

    return true;
}

//note:
//CallWndProc will be executed in the process which myhook.dll injected, not the MySpy process
static LRESULT CALLBACK GetMessageProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code == HC_ACTION) 
    {
        PMSG pMsg = (PMSG)lParam;
        if (pMsg->hwnd  == g_hCaptureWnd)
        {
            ::PostMessage(g_hNotifyWnd, WM_NotifyGetMessage, (WPARAM)pMsg->hwnd, pMsg->message);
            //char szBuf[MAX_PATH] = {0};
            //_snprintf_s(szBuf, MAX_PATH, "GetMessage Handle: 0x%08X PostMsg: %s(%04X), wParam: %08X, lParam: %08X\n", 
            //    pMsg->hwnd, GetMsgStringA(pMsg->message), pMsg->message, (int)pMsg->wParam, (int)pMsg->lParam);
            //OutputDebugStringA(szBuf);
        }
    }

    return CallNextHookEx(g_hGetMessageHook, code, wParam, lParam);
}

DLL_EXPORT bool InstallGetMessageHook(HWND hNotifyWnd, HWND hCaptureWnd)
{
    g_hNotifyWnd = hNotifyWnd;
    g_hCaptureWnd = hCaptureWnd;

    if (!g_hGetMessageHook)
    {
        DWORD dwThreadId = ::GetWindowThreadProcessId(g_hCaptureWnd, NULL);
        g_hGetMessageHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMessageProc, g_hInstance, dwThreadId);

        if (g_hGetMessageHook)
        {
            OutputDebugStringA("Hook GetMessage succeed\n");
            return true;
        }
        else
        {
            DWORD dwError = GetLastError();
            char szError[MAX_PATH];
            _snprintf_s(szError, MAX_PATH, "Hook GetMessage failed, error = %u\n", dwError);
            OutputDebugStringA(szError);
        }
    }

    return false;
}

DLL_EXPORT bool UninstallGetMessageHook()
{
    if (g_hGetMessageHook)
    {
        UnhookWindowsHookEx(g_hGetMessageHook);
        g_hGetMessageHook = NULL;
        OutputDebugStringA("Uninstall GetMessage Hook\n");
    }

    return true;
}

DLL_EXPORT const char* GetMsgStringA(UINT uMsg)
{
    static std::map<UINT, char*> g_mapMsgStr;
    if (0 == g_mapMsgStr.size())
    {
        g_mapMsgStr[ACM_ISPLAYING                      ] = "ACM_ISPLAYING";
        g_mapMsgStr[ACM_OPENA                          ] = "ACM_OPENA";
        g_mapMsgStr[ACM_OPENW                          ] = "ACM_OPENW";
        g_mapMsgStr[ACM_PLAY                           ] = "ACM_PLAY";
        g_mapMsgStr[ACM_STOP                           ] = "ACM_STOP";
        g_mapMsgStr[BCM_GETIDEALSIZE                   ] = "BCM_GETIDEALSIZE";
        g_mapMsgStr[BCM_GETIMAGELIST                   ] = "BCM_GETIMAGELIST";
        g_mapMsgStr[BCM_GETNOTE                        ] = "BCM_GETNOTE";
        g_mapMsgStr[BCM_GETNOTELENGTH                  ] = "BCM_GETNOTELENGTH";
        g_mapMsgStr[BCM_GETSPLITINFO                   ] = "BCM_GETSPLITINFO";
        g_mapMsgStr[BCM_GETTEXTMARGIN                  ] = "BCM_GETTEXTMARGIN";
        g_mapMsgStr[BCM_SETDROPDOWNSTATE               ] = "BCM_SETDROPDOWNSTATE";
        g_mapMsgStr[BCM_SETIMAGELIST                   ] = "BCM_SETIMAGELIST";
        g_mapMsgStr[BCM_SETNOTE                        ] = "BCM_SETNOTE";
        g_mapMsgStr[BCM_SETSHIELD                      ] = "BCM_SETSHIELD";
        g_mapMsgStr[BCM_SETSPLITINFO                   ] = "BCM_SETSPLITINFO";
        g_mapMsgStr[BCM_SETTEXTMARGIN                  ] = "BCM_SETTEXTMARGIN";
        g_mapMsgStr[BM_CLICK                           ] = "BM_CLICK";
        g_mapMsgStr[BM_GETCHECK                        ] = "BM_GETCHECK";
        g_mapMsgStr[BM_GETIMAGE                        ] = "BM_GETIMAGE";
        g_mapMsgStr[BM_GETSTATE                        ] = "BM_GETSTATE";
        g_mapMsgStr[BM_SETCHECK                        ] = "BM_SETCHECK";
        g_mapMsgStr[BM_SETDONTCLICK                    ] = "BM_SETDONTCLICK";
        g_mapMsgStr[BM_SETIMAGE                        ] = "BM_SETIMAGE";
        g_mapMsgStr[BM_SETSTATE                        ] = "BM_SETSTATE";
        g_mapMsgStr[BM_SETSTYLE                        ] = "BM_SETSTYLE";
        g_mapMsgStr[CB_ADDSTRING                       ] = "CB_ADDSTRING";
        g_mapMsgStr[CB_DELETESTRING                    ] = "CB_DELETESTRING";
        g_mapMsgStr[CB_DIR                             ] = "CB_DIR";
        g_mapMsgStr[CB_FINDSTRING                      ] = "CB_FINDSTRING";
        g_mapMsgStr[CB_FINDSTRINGEXACT                 ] = "CB_FINDSTRINGEXACT";
        g_mapMsgStr[CB_GETCOMBOBOXINFO                 ] = "CB_GETCOMBOBOXINFO";
        g_mapMsgStr[CB_GETCOUNT                        ] = "CB_GETCOUNT";
        g_mapMsgStr[CB_GETCUEBANNER                    ] = "CB_GETCUEBANNER";
        g_mapMsgStr[CB_GETCURSEL                       ] = "CB_GETCURSEL";
        g_mapMsgStr[CB_GETDROPPEDCONTROLRECT           ] = "CB_GETDROPPEDCONTROLRECT";
        g_mapMsgStr[CB_GETDROPPEDSTATE                 ] = "CB_GETDROPPEDSTATE";
        g_mapMsgStr[CB_GETDROPPEDWIDTH                 ] = "CB_GETDROPPEDWIDTH";
        g_mapMsgStr[CB_GETEDITSEL                      ] = "CB_GETEDITSEL";
        g_mapMsgStr[CB_GETEXTENDEDUI                   ] = "CB_GETEXTENDEDUI";
        g_mapMsgStr[CB_GETHORIZONTALEXTENT             ] = "CB_GETHORIZONTALEXTENT";
        g_mapMsgStr[CB_GETITEMDATA                     ] = "CB_GETITEMDATA";
        g_mapMsgStr[CB_GETITEMHEIGHT                   ] = "CB_GETITEMHEIGHT";
        g_mapMsgStr[CB_GETLBTEXT                       ] = "CB_GETLBTEXT";
        g_mapMsgStr[CB_GETLBTEXTLEN                    ] = "CB_GETLBTEXTLEN";
        g_mapMsgStr[CB_GETLOCALE                       ] = "CB_GETLOCALE";
        g_mapMsgStr[CB_GETMINVISIBLE                   ] = "CB_GETMINVISIBLE";
        g_mapMsgStr[CB_GETTOPINDEX                     ] = "CB_GETTOPINDEX";
        g_mapMsgStr[CB_INITSTORAGE                     ] = "CB_INITSTORAGE";
        g_mapMsgStr[CB_INSERTSTRING                    ] = "CB_INSERTSTRING";
        g_mapMsgStr[CB_LIMITTEXT                       ] = "CB_LIMITTEXT";
        g_mapMsgStr[CB_MULTIPLEADDSTRING               ] = "CB_MULTIPLEADDSTRING";
        g_mapMsgStr[CB_RESETCONTENT                    ] = "CB_RESETCONTENT";
        g_mapMsgStr[CB_SELECTSTRING                    ] = "CB_SELECTSTRING";
        g_mapMsgStr[CB_SETCUEBANNER                    ] = "CB_SETCUEBANNER";
        g_mapMsgStr[CB_SETCURSEL                       ] = "CB_SETCURSEL";
        g_mapMsgStr[CB_SETDROPPEDWIDTH                 ] = "CB_SETDROPPEDWIDTH";
        g_mapMsgStr[CB_SETEDITSEL                      ] = "CB_SETEDITSEL";
        g_mapMsgStr[CB_SETEXTENDEDUI                   ] = "CB_SETEXTENDEDUI";
        g_mapMsgStr[CB_SETHORIZONTALEXTENT             ] = "CB_SETHORIZONTALEXTENT";
        g_mapMsgStr[CB_SETITEMDATA                     ] = "CB_SETITEMDATA";
        g_mapMsgStr[CB_SETITEMHEIGHT                   ] = "CB_SETITEMHEIGHT";
        g_mapMsgStr[CB_SETLOCALE                       ] = "CB_SETLOCALE";
        g_mapMsgStr[CB_SETMINVISIBLE                   ] = "CB_SETMINVISIBLE";
        g_mapMsgStr[CB_SETTOPINDEX                     ] = "CB_SETTOPINDEX";
        g_mapMsgStr[CB_SHOWDROPDOWN                    ] = "CB_SHOWDROPDOWN";
        g_mapMsgStr[CBEM_GETCOMBOCONTROL               ] = "CBEM_GETCOMBOCONTROL";
        g_mapMsgStr[CBEM_GETEDITCONTROL                ] = "CBEM_GETEDITCONTROL";
        g_mapMsgStr[CBEM_GETEXTENDEDSTYLE              ] = "CBEM_GETEXTENDEDSTYLE";
        g_mapMsgStr[CBEM_GETIMAGELIST                  ] = "CBEM_GETIMAGELIST";
        g_mapMsgStr[CBEM_GETITEMA                      ] = "CBEM_GETITEMA";
        g_mapMsgStr[CBEM_GETITEMW                      ] = "CBEM_GETITEMW";
        g_mapMsgStr[CBEM_HASEDITCHANGED                ] = "CBEM_HASEDITCHANGED";
        g_mapMsgStr[CBEM_INSERTITEMA                   ] = "CBEM_INSERTITEMA";
        g_mapMsgStr[CBEM_INSERTITEMW                   ] = "CBEM_INSERTITEMW";
        g_mapMsgStr[CBEM_SETEXSTYLE                    ] = "CBEM_SETEXSTYLE";
        g_mapMsgStr[CBEM_SETEXTENDEDSTYLE              ] = "CBEM_SETEXTENDEDSTYLE";
        g_mapMsgStr[CBEM_SETIMAGELIST                  ] = "CBEM_SETIMAGELIST";
        g_mapMsgStr[CBEM_SETITEMA                      ] = "CBEM_SETITEMA";
        g_mapMsgStr[CBEM_SETITEMW                      ] = "CBEM_SETITEMW";
        g_mapMsgStr[CDM_GETFILEPATH                    ] = "CDM_GETFILEPATH";
        g_mapMsgStr[CDM_GETFOLDERIDLIST                ] = "CDM_GETFOLDERIDLIST";
        g_mapMsgStr[CDM_GETFOLDERPATH                  ] = "CDM_GETFOLDERPATH";
        g_mapMsgStr[CDM_GETSPEC                        ] = "CDM_GETSPEC";
        g_mapMsgStr[CDM_HIDECONTROL                    ] = "CDM_HIDECONTROL";
        g_mapMsgStr[CDM_SETCONTROLTEXT                 ] = "CDM_SETCONTROLTEXT";
        g_mapMsgStr[CDM_SETDEFEXT                      ] = "CDM_SETDEFEXT";
        g_mapMsgStr[CCM_DPISCALE                       ] = "CCM_DPISCALE";
        g_mapMsgStr[CCM_GETCOLORSCHEME                 ] = "CCM_GETCOLORSCHEME";
        g_mapMsgStr[CCM_GETDROPTARGET                  ] = "CCM_GETDROPTARGET";
        g_mapMsgStr[CCM_GETUNICODEFORMAT               ] = "CCM_GETUNICODEFORMAT";
        g_mapMsgStr[CCM_GETVERSION                     ] = "CCM_GETVERSION";
        g_mapMsgStr[CCM_SETBKCOLOR                     ] = "CCM_SETBKCOLOR";
        g_mapMsgStr[CCM_SETCOLORSCHEME                 ] = "CCM_SETCOLORSCHEME";
        g_mapMsgStr[CCM_SETNOTIFYWINDOW                ] = "CCM_SETNOTIFYWINDOW";
        g_mapMsgStr[CCM_SETUNICODEFORMAT               ] = "CCM_SETUNICODEFORMAT";
        g_mapMsgStr[CCM_SETVERSION                     ] = "CCM_SETVERSION";
        g_mapMsgStr[CCM_SETWINDOWTHEME                 ] = "CCM_SETWINDOWTHEME";
        g_mapMsgStr[DL_BEGINDRAG                       ] = "DL_BEGINDRAG";
        g_mapMsgStr[DL_CANCELDRAG                      ] = "DL_CANCELDRAG";
        g_mapMsgStr[DL_DRAGGING                        ] = "DL_DRAGGING";
        g_mapMsgStr[DL_DROPPED                         ] = "DL_DROPPED";
        g_mapMsgStr[DM_GETDEFID                        ] = "DM_GETDEFID";
        g_mapMsgStr[DM_REPOSITION                      ] = "DM_REPOSITION";
        g_mapMsgStr[DM_SETDEFID                        ] = "DM_SETDEFID";
        g_mapMsgStr[DTM_CLOSEMONTHCAL                  ] = "DTM_CLOSEMONTHCAL";
        g_mapMsgStr[DTM_GETDATETIMEPICKERINFO          ] = "DTM_GETDATETIMEPICKERINFO";
        g_mapMsgStr[DTM_GETIDEALSIZE                   ] = "DTM_GETIDEALSIZE";
        g_mapMsgStr[DTM_GETMCCOLOR                     ] = "DTM_GETMCCOLOR";
        g_mapMsgStr[DTM_GETMCFONT                      ] = "DTM_GETMCFONT";
        g_mapMsgStr[DTM_GETMCSTYLE                     ] = "DTM_GETMCSTYLE";
        g_mapMsgStr[DTM_GETMONTHCAL                    ] = "DTM_GETMONTHCAL";
        g_mapMsgStr[DTM_GETRANGE                       ] = "DTM_GETRANGE";
        g_mapMsgStr[DTM_GETSYSTEMTIME                  ] = "DTM_GETSYSTEMTIME";
        g_mapMsgStr[DTM_SETFORMATA                     ] = "DTM_SETFORMATA";
        g_mapMsgStr[DTM_SETFORMATW                     ] = "DTM_SETFORMATW";
        g_mapMsgStr[DTM_SETMCCOLOR                     ] = "DTM_SETMCCOLOR";
        g_mapMsgStr[DTM_SETMCFONT                      ] = "DTM_SETMCFONT";
        g_mapMsgStr[DTM_SETMCSTYLE                     ] = "DTM_SETMCSTYLE";
        g_mapMsgStr[DTM_SETRANGE                       ] = "DTM_SETRANGE";
        g_mapMsgStr[DTM_SETSYSTEMTIME                  ] = "DTM_SETSYSTEMTIME";
        g_mapMsgStr[EM_AUTOURLDETECT                   ] = "EM_AUTOURLDETECT";
        g_mapMsgStr[EM_CANPASTE                        ] = "EM_CANPASTE";
        g_mapMsgStr[EM_CANREDO                         ] = "EM_CANREDO";
        g_mapMsgStr[EM_CANUNDO                         ] = "EM_CANUNDO";
        g_mapMsgStr[EM_CHARFROMPOS                     ] = "EM_CHARFROMPOS";
        g_mapMsgStr[EM_CONVPOSITION                    ] = "EM_CONVPOSITION";
        g_mapMsgStr[EM_DISPLAYBAND                     ] = "EM_DISPLAYBAND";
        g_mapMsgStr[EM_EMPTYUNDOBUFFER                 ] = "EM_EMPTYUNDOBUFFER";
        g_mapMsgStr[EM_EXGETSEL                        ] = "EM_EXGETSEL";
        g_mapMsgStr[EM_EXLIMITTEXT                     ] = "EM_EXLIMITTEXT";
        g_mapMsgStr[EM_EXLINEFROMCHAR                  ] = "EM_EXLINEFROMCHAR";
        g_mapMsgStr[EM_EXSETSEL                        ] = "EM_EXSETSEL";
        g_mapMsgStr[EM_FINDTEXT                        ] = "EM_FINDTEXT";
        g_mapMsgStr[EM_FINDTEXTW                       ] = "EM_FINDTEXTW";
        g_mapMsgStr[EM_FINDTEXTEX                      ] = "EM_FINDTEXTEX";
        g_mapMsgStr[EM_FINDTEXTEXW                     ] = "EM_FINDTEXTEXW";
        g_mapMsgStr[EM_FINDWORDBREAK                   ] = "EM_FINDWORDBREAK";
        g_mapMsgStr[EM_FMTLINES                        ] = "EM_FMTLINES";
        g_mapMsgStr[EM_FORMATRANGE                     ] = "EM_FORMATRANGE";
        g_mapMsgStr[EM_GETAUTOURLDETECT                ] = "EM_GETAUTOURLDETECT";
        g_mapMsgStr[EM_GETBIDIOPTIONS                  ] = "EM_GETBIDIOPTIONS";
        g_mapMsgStr[EM_GETCHARFORMAT                   ] = "EM_GETCHARFORMAT";
        g_mapMsgStr[EM_GETCTFMODEBIAS                  ] = "EM_GETCTFMODEBIAS";
        g_mapMsgStr[EM_GETCTFOPENSTATUS                ] = "EM_GETCTFOPENSTATUS";
        g_mapMsgStr[EM_GETCUEBANNER                    ] = "EM_GETCUEBANNER";
        g_mapMsgStr[EM_GETEDITSTYLE                    ] = "EM_GETEDITSTYLE";
        g_mapMsgStr[EM_GETEVENTMASK                    ] = "EM_GETEVENTMASK";
        g_mapMsgStr[EM_GETFIRSTVISIBLELINE             ] = "EM_GETFIRSTVISIBLELINE";
        g_mapMsgStr[EM_GETHANDLE                       ] = "EM_GETHANDLE";
        g_mapMsgStr[EM_GETHILITE                       ] = "EM_GETHILITE";
        g_mapMsgStr[EM_GETHYPHENATEINFO                ] = "EM_GETHYPHENATEINFO";
        g_mapMsgStr[EM_GETIMECOLOR                     ] = "EM_GETIMECOLOR";
        g_mapMsgStr[EM_GETIMECOMPMODE                  ] = "EM_GETIMECOMPMODE";
        g_mapMsgStr[EM_GETIMECOMPTEXT                  ] = "EM_GETIMECOMPTEXT";
        g_mapMsgStr[EM_GETIMEMODEBIAS                  ] = "EM_GETIMEMODEBIAS";
        g_mapMsgStr[EM_GETIMEOPTIONS                   ] = "EM_GETIMEOPTIONS";
        g_mapMsgStr[EM_GETIMEPROPERTY                  ] = "EM_GETIMEPROPERTY";
        g_mapMsgStr[EM_GETIMESTATUS                    ] = "EM_GETIMESTATUS";
        g_mapMsgStr[EM_GETLANGOPTIONS                  ] = "EM_GETLANGOPTIONS";
        g_mapMsgStr[EM_GETLIMITTEXT                    ] = "EM_GETLIMITTEXT";
        g_mapMsgStr[EM_GETLINE                         ] = "EM_GETLINE";
        g_mapMsgStr[EM_GETLINECOUNT                    ] = "EM_GETLINECOUNT";
        g_mapMsgStr[EM_GETMARGINS                      ] = "EM_GETMARGINS";
        g_mapMsgStr[EM_GETMODIFY                       ] = "EM_GETMODIFY";
        g_mapMsgStr[EM_GETOLEINTERFACE                 ] = "EM_GETOLEINTERFACE";
        g_mapMsgStr[EM_GETOPTIONS                      ] = "EM_GETOPTIONS";
        g_mapMsgStr[EM_GETPAGE                         ] = "EM_GETPAGE";
        g_mapMsgStr[EM_GETPAGEROTATE                   ] = "EM_GETPAGEROTATE";
        g_mapMsgStr[EM_GETPARAFORMAT                   ] = "EM_GETPARAFORMAT";
        g_mapMsgStr[EM_GETPASSWORDCHAR                 ] = "EM_GETPASSWORDCHAR";
        g_mapMsgStr[EM_GETPUNCTUATION                  ] = "EM_GETPUNCTUATION";
        g_mapMsgStr[EM_GETQUERYRTFOBJ                  ] = "EM_GETQUERYRTFOBJ";
        g_mapMsgStr[EM_GETRECT                         ] = "EM_GETRECT";
        g_mapMsgStr[EM_GETREDONAME                     ] = "EM_GETREDONAME";
        g_mapMsgStr[EM_GETSCROLLPOS                    ] = "EM_GETSCROLLPOS";
        g_mapMsgStr[EM_GETSEL                          ] = "EM_GETSEL";
        g_mapMsgStr[EM_GETSELTEXT                      ] = "EM_GETSELTEXT";
        g_mapMsgStr[EM_GETTEXTEX                       ] = "EM_GETTEXTEX";
        g_mapMsgStr[EM_GETTEXTLENGTHEX                 ] = "EM_GETTEXTLENGTHEX";
        g_mapMsgStr[EM_GETTEXTMODE                     ] = "EM_GETTEXTMODE";
        g_mapMsgStr[EM_GETTEXTRANGE                    ] = "EM_GETTEXTRANGE";
        g_mapMsgStr[EM_GETTHUMB                        ] = "EM_GETTHUMB";
        g_mapMsgStr[EM_GETTYPOGRAPHYOPTIONS            ] = "EM_GETTYPOGRAPHYOPTIONS";
        g_mapMsgStr[EM_GETUNDONAME                     ] = "EM_GETUNDONAME";
        g_mapMsgStr[EM_GETVIEWKIND                     ] = "EM_GETVIEWKIND";
        g_mapMsgStr[EM_GETWORDBREAKPROC                ] = "EM_GETWORDBREAKPROC";
        g_mapMsgStr[EM_GETWORDBREAKPROCEX              ] = "EM_GETWORDBREAKPROCEX";
        g_mapMsgStr[EM_GETWORDWRAPMODE                 ] = "EM_GETWORDWRAPMODE";
        g_mapMsgStr[EM_GETZOOM                         ] = "EM_GETZOOM";
        g_mapMsgStr[EM_HIDEBALLOONTIP                  ] = "EM_HIDEBALLOONTIP";
        g_mapMsgStr[EM_HIDESELECTION                   ] = "EM_HIDESELECTION";
        g_mapMsgStr[EM_ISIME                           ] = "EM_ISIME";
        g_mapMsgStr[EM_LINEFROMCHAR                    ] = "EM_LINEFROMCHAR";
        g_mapMsgStr[EM_LINEINDEX                       ] = "EM_LINEINDEX";
        g_mapMsgStr[EM_LINELENGTH                      ] = "EM_LINELENGTH";
        g_mapMsgStr[EM_LINESCROLL                      ] = "EM_LINESCROLL";
        g_mapMsgStr[EM_NOSETFOCUS                      ] = "EM_NOSETFOCUS";
        g_mapMsgStr[EM_OUTLINE                         ] = "EM_OUTLINE";
        g_mapMsgStr[EM_PASTESPECIAL                    ] = "EM_PASTESPECIAL";
        g_mapMsgStr[EM_POSFROMCHAR                     ] = "EM_POSFROMCHAR";
        g_mapMsgStr[EM_RECONVERSION                    ] = "EM_RECONVERSION";
        g_mapMsgStr[EM_REDO                            ] = "EM_REDO";
        g_mapMsgStr[EM_REPLACESEL                      ] = "EM_REPLACESEL";
        g_mapMsgStr[EM_REQUESTRESIZE                   ] = "EM_REQUESTRESIZE";
        g_mapMsgStr[EM_SCROLL                          ] = "EM_SCROLL";
        g_mapMsgStr[EM_SCROLLCARET                     ] = "EM_SCROLLCARET";
        g_mapMsgStr[EM_SELECTIONTYPE                   ] = "EM_SELECTIONTYPE";
        g_mapMsgStr[EM_SETBIDIOPTIONS                  ] = "EM_SETBIDIOPTIONS";
        g_mapMsgStr[EM_SETBKGNDCOLOR                   ] = "EM_SETBKGNDCOLOR";
        g_mapMsgStr[EM_SETCHARFORMAT                   ] = "EM_SETCHARFORMAT";
        g_mapMsgStr[EM_SETCTFMODEBIAS                  ] = "EM_SETCTFMODEBIAS";
        g_mapMsgStr[EM_SETCTFOPENSTATUS                ] = "EM_SETCTFOPENSTATUS";
        g_mapMsgStr[EM_SETCUEBANNER                    ] = "EM_SETCUEBANNER";
        g_mapMsgStr[EM_SETEDITSTYLE                    ] = "EM_SETEDITSTYLE";
        g_mapMsgStr[EM_SETEVENTMASK                    ] = "EM_SETEVENTMASK";
        g_mapMsgStr[EM_SETFONTSIZE                     ] = "EM_SETFONTSIZE";
        g_mapMsgStr[EM_SETHANDLE                       ] = "EM_SETHANDLE";
        g_mapMsgStr[EM_SETHILITE                       ] = "EM_SETHILITE";
        g_mapMsgStr[EM_SETHYPHENATEINFO                ] = "EM_SETHYPHENATEINFO";
        g_mapMsgStr[EM_SETIMECOLOR                     ] = "EM_SETIMECOLOR";
        g_mapMsgStr[EM_SETIMEMODEBIAS                  ] = "EM_SETIMEMODEBIAS";
        g_mapMsgStr[EM_SETIMEOPTIONS                   ] = "EM_SETIMEOPTIONS";
        g_mapMsgStr[EM_SETIMESTATUS                    ] = "EM_SETIMESTATUS";
        g_mapMsgStr[EM_SETLANGOPTIONS                  ] = "EM_SETLANGOPTIONS";
        g_mapMsgStr[EM_SETLIMITTEXT                    ] = "EM_SETLIMITTEXT";
        g_mapMsgStr[EM_SETMARGINS                      ] = "EM_SETMARGINS";
        g_mapMsgStr[EM_SETMODIFY                       ] = "EM_SETMODIFY";
        g_mapMsgStr[EM_SETOLECALLBACK                  ] = "EM_SETOLECALLBACK";
        g_mapMsgStr[EM_SETOPTIONS                      ] = "EM_SETOPTIONS";
        g_mapMsgStr[EM_SETPAGE                         ] = "EM_SETPAGE";
        g_mapMsgStr[EM_SETPAGEROTATE                   ] = "EM_SETPAGEROTATE";
        g_mapMsgStr[EM_SETPALETTE                      ] = "EM_SETPALETTE";
        g_mapMsgStr[EM_SETPARAFORMAT                   ] = "EM_SETPARAFORMAT";
        g_mapMsgStr[EM_SETPASSWORDCHAR                 ] = "EM_SETPASSWORDCHAR";
        g_mapMsgStr[EM_SETPUNCTUATION                  ] = "EM_SETPUNCTUATION";
        g_mapMsgStr[EM_SETQUERYRTFOBJ                  ] = "EM_SETQUERYRTFOBJ";
        g_mapMsgStr[EM_SETREADONLY                     ] = "EM_SETREADONLY";
        g_mapMsgStr[EM_SETRECT                         ] = "EM_SETRECT";
        g_mapMsgStr[EM_SETRECTNP                       ] = "EM_SETRECTNP";
        g_mapMsgStr[EM_SETSCROLLPOS                    ] = "EM_SETSCROLLPOS";
        g_mapMsgStr[EM_SETSEL                          ] = "EM_SETSEL";
        g_mapMsgStr[EM_SETTABSTOPS                     ] = "EM_SETTABSTOPS";
        g_mapMsgStr[EM_SETTARGETDEVICE                 ] = "EM_SETTARGETDEVICE";
        g_mapMsgStr[EM_SETTEXTEX                       ] = "EM_SETTEXTEX";
        g_mapMsgStr[EM_SETTEXTMODE                     ] = "EM_SETTEXTMODE";
        g_mapMsgStr[EM_SETTYPOGRAPHYOPTIONS            ] = "EM_SETTYPOGRAPHYOPTIONS";
        g_mapMsgStr[EM_SETUNDOLIMIT                    ] = "EM_SETUNDOLIMIT";
        g_mapMsgStr[EM_SETVIEWKIND                     ] = "EM_SETVIEWKIND";
        g_mapMsgStr[EM_SETWORDBREAKPROC                ] = "EM_SETWORDBREAKPROC";
        g_mapMsgStr[EM_SETWORDBREAKPROCEX              ] = "EM_SETWORDBREAKPROCEX";
        g_mapMsgStr[EM_SETWORDWRAPMODE                 ] = "EM_SETWORDWRAPMODE";
        g_mapMsgStr[EM_SETZOOM                         ] = "EM_SETZOOM";
        g_mapMsgStr[EM_SHOWBALLOONTIP                  ] = "EM_SHOWBALLOONTIP";
        g_mapMsgStr[EM_SHOWSCROLLBAR                   ] = "EM_SHOWSCROLLBAR";
        g_mapMsgStr[EM_STOPGROUPTYPING                 ] = "EM_STOPGROUPTYPING";
        g_mapMsgStr[EM_STREAMIN                        ] = "EM_STREAMIN";
        g_mapMsgStr[EM_STREAMOUT                       ] = "EM_STREAMOUT";
        g_mapMsgStr[EM_TAKEFOCUS                       ] = "EM_TAKEFOCUS";
        g_mapMsgStr[EM_UNDO                            ] = "EM_UNDO";
        g_mapMsgStr[HDM_CREATEDRAGIMAGE                ] = "HDM_CREATEDRAGIMAGE";
        g_mapMsgStr[HDM_CLEARFILTER                    ] = "HDM_CLEARFILTER";
        g_mapMsgStr[HDM_DELETEITEM                     ] = "HDM_DELETEITEM";
        g_mapMsgStr[HDM_EDITFILTER                     ] = "HDM_EDITFILTER";
        g_mapMsgStr[HDM_GETBITMAPMARGIN                ] = "HDM_GETBITMAPMARGIN";
        g_mapMsgStr[HDM_GETFOCUSEDITEM                 ] = "HDM_GETFOCUSEDITEM";
        g_mapMsgStr[HDM_GETIMAGELIST                   ] = "HDM_GETIMAGELIST";
        g_mapMsgStr[HDM_GETITEMA                       ] = "HDM_GETITEMA";
        g_mapMsgStr[HDM_GETITEMW                       ] = "HDM_GETITEMW";
        g_mapMsgStr[HDM_GETITEMCOUNT                   ] = "HDM_GETITEMCOUNT";
        g_mapMsgStr[HDM_GETITEMDROPDOWNRECT            ] = "HDM_GETITEMDROPDOWNRECT";
        g_mapMsgStr[HDM_GETITEMRECT                    ] = "HDM_GETITEMRECT";
        g_mapMsgStr[HDM_GETORDERARRAY                  ] = "HDM_GETORDERARRAY";
        g_mapMsgStr[HDM_GETOVERFLOWRECT                ] = "HDM_GETOVERFLOWRECT";
        g_mapMsgStr[HDM_HITTEST                        ] = "HDM_HITTEST";
        g_mapMsgStr[HDM_INSERTITEMA                    ] = "HDM_INSERTITEMA";
        g_mapMsgStr[HDM_INSERTITEMW                    ] = "HDM_INSERTITEMW";
        g_mapMsgStr[HDM_LAYOUT                         ] = "HDM_LAYOUT";
        g_mapMsgStr[HDM_ORDERTOINDEX                   ] = "HDM_ORDERTOINDEX";
        g_mapMsgStr[HDM_SETBITMAPMARGIN                ] = "HDM_SETBITMAPMARGIN";
        g_mapMsgStr[HDM_SETFILTERCHANGETIMEOUT         ] = "HDM_SETFILTERCHANGETIMEOUT";
        g_mapMsgStr[HDM_SETFOCUSEDITEM                 ] = "HDM_SETFOCUSEDITEM";
        g_mapMsgStr[HDM_SETHOTDIVIDER                  ] = "HDM_SETHOTDIVIDER";
        g_mapMsgStr[HDM_SETIMAGELIST                   ] = "HDM_SETIMAGELIST";
        g_mapMsgStr[HDM_SETITEMA                       ] = "HDM_SETITEMA";
        g_mapMsgStr[HDM_SETITEMW                       ] = "HDM_SETITEMW";
        g_mapMsgStr[HDM_SETORDERARRAY                  ] = "HDM_SETORDERARRAY";
        g_mapMsgStr[HKM_GETHOTKEY                      ] = "HKM_GETHOTKEY";
        g_mapMsgStr[HKM_SETHOTKEY                      ] = "HKM_SETHOTKEY";
        g_mapMsgStr[HKM_SETRULES                       ] = "HKM_SETRULES";
        g_mapMsgStr[IPM_CLEARADDRESS                   ] = "IPM_CLEARADDRESS";
        g_mapMsgStr[IPM_GETADDRESS                     ] = "IPM_GETADDRESS";
        g_mapMsgStr[IPM_ISBLANK                        ] = "IPM_ISBLANK";
        g_mapMsgStr[IPM_SETADDRESS                     ] = "IPM_SETADDRESS";
        g_mapMsgStr[IPM_SETFOCUS                       ] = "IPM_SETFOCUS";
        g_mapMsgStr[IPM_SETRANGE                       ] = "IPM_SETRANGE";
        g_mapMsgStr[LB_ADDFILE                         ] = "LB_ADDFILE";
        g_mapMsgStr[LB_ADDSTRING                       ] = "LB_ADDSTRING";
        g_mapMsgStr[LB_DELETESTRING                    ] = "LB_DELETESTRING";
        g_mapMsgStr[LB_DIR                             ] = "LB_DIR";
        g_mapMsgStr[LB_FINDSTRING                      ] = "LB_FINDSTRING";
        g_mapMsgStr[LB_FINDSTRINGEXACT                 ] = "LB_FINDSTRINGEXACT";
        g_mapMsgStr[LB_GETANCHORINDEX                  ] = "LB_GETANCHORINDEX";
        g_mapMsgStr[LB_GETCARETINDEX                   ] = "LB_GETCARETINDEX";
        g_mapMsgStr[LB_GETCOUNT                        ] = "LB_GETCOUNT";
        g_mapMsgStr[LB_GETCURSEL                       ] = "LB_GETCURSEL";
        g_mapMsgStr[LB_GETHORIZONTALEXTENT             ] = "LB_GETHORIZONTALEXTENT";
        g_mapMsgStr[LB_GETITEMDATA                     ] = "LB_GETITEMDATA";
        g_mapMsgStr[LB_GETITEMHEIGHT                   ] = "LB_GETITEMHEIGHT";
        g_mapMsgStr[LB_GETITEMRECT                     ] = "LB_GETITEMRECT";
        g_mapMsgStr[LB_GETLISTBOXINFO                  ] = "LB_GETLISTBOXINFO";
        g_mapMsgStr[LB_GETLOCALE                       ] = "LB_GETLOCALE";
        g_mapMsgStr[LB_GETSEL                          ] = "LB_GETSEL";
        g_mapMsgStr[LB_GETSELCOUNT                     ] = "LB_GETSELCOUNT";
        g_mapMsgStr[LB_GETSELITEMS                     ] = "LB_GETSELITEMS";
        g_mapMsgStr[LB_GETTEXT                         ] = "LB_GETTEXT";
        g_mapMsgStr[LB_GETTEXTLEN                      ] = "LB_GETTEXTLEN";
        g_mapMsgStr[LB_GETTOPINDEX                     ] = "LB_GETTOPINDEX";
        g_mapMsgStr[LB_INITSTORAGE                     ] = "LB_INITSTORAGE";
        g_mapMsgStr[LB_INSERTSTRING                    ] = "LB_INSERTSTRING";
        g_mapMsgStr[LB_ITEMFROMPOINT                   ] = "LB_ITEMFROMPOINT";
        g_mapMsgStr[LB_MULTIPLEADDSTRING               ] = "LB_MULTIPLEADDSTRING";
        g_mapMsgStr[LB_RESETCONTENT                    ] = "LB_RESETCONTENT";
        g_mapMsgStr[LB_SELECTSTRING                    ] = "LB_SELECTSTRING";
        g_mapMsgStr[LB_SELITEMRANGE                    ] = "LB_SELITEMRANGE";
        g_mapMsgStr[LB_SELITEMRANGEEX                  ] = "LB_SELITEMRANGEEX";
        g_mapMsgStr[LB_SETANCHORINDEX                  ] = "LB_SETANCHORINDEX";
        g_mapMsgStr[LB_SETCARETINDEX                   ] = "LB_SETCARETINDEX";
        g_mapMsgStr[LB_SETCOLUMNWIDTH                  ] = "LB_SETCOLUMNWIDTH";
        g_mapMsgStr[LB_SETCOUNT                        ] = "LB_SETCOUNT";
        g_mapMsgStr[LB_SETCURSEL                       ] = "LB_SETCURSEL";
        g_mapMsgStr[LB_SETHORIZONTALEXTENT             ] = "LB_SETHORIZONTALEXTENT";
        g_mapMsgStr[LB_SETITEMDATA                     ] = "LB_SETITEMDATA";
        g_mapMsgStr[LB_SETITEMHEIGHT                   ] = "LB_SETITEMHEIGHT";
        g_mapMsgStr[LB_SETLOCALE                       ] = "LB_SETLOCALE";
        g_mapMsgStr[LB_SETSEL                          ] = "LB_SETSEL";
        g_mapMsgStr[LB_SETTABSTOPS                     ] = "LB_SETTABSTOPS";
        g_mapMsgStr[LB_SETTOPINDEX                     ] = "LB_SETTOPINDEX";
        g_mapMsgStr[LM_GETIDEALHEIGHT                  ] = "LM_GETIDEALHEIGHT";
        g_mapMsgStr[LM_GETITEM                         ] = "LM_GETITEM";
        g_mapMsgStr[LM_HITTEST                         ] = "LM_HITTEST";
        g_mapMsgStr[LM_SETITEM                         ] = "LM_SETITEM";
        g_mapMsgStr[LVM_APPROXIMATEVIEWRECT            ] = "LVM_APPROXIMATEVIEWRECT";
        g_mapMsgStr[LVM_ARRANGE                        ] = "LVM_ARRANGE";
        g_mapMsgStr[LVM_CANCELEDITLABEL                ] = "LVM_CANCELEDITLABEL";
        g_mapMsgStr[LVM_CREATEDRAGIMAGE                ] = "LVM_CREATEDRAGIMAGE";
        g_mapMsgStr[LVM_DELETEALLITEMS                 ] = "LVM_DELETEALLITEMS";
        g_mapMsgStr[LVM_DELETECOLUMN                   ] = "LVM_DELETECOLUMN";
        g_mapMsgStr[LVM_DELETEITEM                     ] = "LVM_DELETEITEM";
        g_mapMsgStr[LVM_EDITLABELA                     ] = "LVM_EDITLABELA";
        g_mapMsgStr[LVM_EDITLABELW                     ] = "LVM_EDITLABELW";
        g_mapMsgStr[LVM_ENABLEGROUPVIEW                ] = "LVM_ENABLEGROUPVIEW";
        g_mapMsgStr[LVM_ENSUREVISIBLE                  ] = "LVM_ENSUREVISIBLE";
        g_mapMsgStr[LVM_FINDITEMA                      ] = "LVM_FINDITEMA";
        g_mapMsgStr[LVM_FINDITEMW                      ] = "LVM_FINDITEMW";
        g_mapMsgStr[LVM_GETBKCOLOR                     ] = "LVM_GETBKCOLOR";
        g_mapMsgStr[LVM_GETBKIMAGEA                    ] = "LVM_GETBKIMAGEA";
        g_mapMsgStr[LVM_GETBKIMAGEW                    ] = "LVM_GETBKIMAGEW";
        g_mapMsgStr[LVM_GETCALLBACKMASK                ] = "LVM_GETCALLBACKMASK";
        g_mapMsgStr[LVM_GETCOLUMNA                     ] = "LVM_GETCOLUMNA";
        g_mapMsgStr[LVM_GETCOLUMNW                     ] = "LVM_GETCOLUMNW";
        g_mapMsgStr[LVM_GETCOLUMNORDERARRAY            ] = "LVM_GETCOLUMNORDERARRAY";
        g_mapMsgStr[LVM_GETCOLUMNWIDTH                 ] = "LVM_GETCOLUMNWIDTH";
        g_mapMsgStr[LVM_GETCOUNTPERPAGE                ] = "LVM_GETCOUNTPERPAGE";
        g_mapMsgStr[LVM_GETEDITCONTROL                 ] = "LVM_GETEDITCONTROL";
        g_mapMsgStr[LVM_GETEMPTYTEXT                   ] = "LVM_GETEMPTYTEXT";
        g_mapMsgStr[LVM_GETEXTENDEDLISTVIEWSTYLE       ] = "LVM_GETEXTENDEDLISTVIEWSTYLE";
        g_mapMsgStr[LVM_GETFOCUSEDGROUP                ] = "LVM_GETFOCUSEDGROUP";
        g_mapMsgStr[LVM_GETFOOTERINFO                  ] = "LVM_GETFOOTERINFO";
        g_mapMsgStr[LVM_GETFOOTERITEM                  ] = "LVM_GETFOOTERITEM";
        g_mapMsgStr[LVM_GETFOOTERITEMRECT              ] = "LVM_GETFOOTERITEMRECT";
        g_mapMsgStr[LVM_GETFOOTERRECT                  ] = "LVM_GETFOOTERRECT";
        g_mapMsgStr[LVM_GETGROUPCOUNT                  ] = "LVM_GETGROUPCOUNT";
        g_mapMsgStr[LVM_GETGROUPINFO                   ] = "LVM_GETGROUPINFO";
        g_mapMsgStr[LVM_GETGROUPINFOBYINDEX            ] = "LVM_GETGROUPINFOBYINDEX";
        g_mapMsgStr[LVM_GETGROUPMETRICS                ] = "LVM_GETGROUPMETRICS";
        g_mapMsgStr[LVM_GETGROUPRECT                   ] = "LVM_GETGROUPRECT";
        g_mapMsgStr[LVM_GETGROUPSTATE                  ] = "LVM_GETGROUPSTATE";
        g_mapMsgStr[LVM_GETHEADER                      ] = "LVM_GETHEADER";
        g_mapMsgStr[LVM_GETHOTCURSOR                   ] = "LVM_GETHOTCURSOR";
        g_mapMsgStr[LVM_GETHOTITEM                     ] = "LVM_GETHOTITEM";
        g_mapMsgStr[LVM_GETHOVERTIME                   ] = "LVM_GETHOVERTIME";
        g_mapMsgStr[LVM_GETIMAGELIST                   ] = "LVM_GETIMAGELIST";
        g_mapMsgStr[LVM_GETINSERTMARK                  ] = "LVM_GETINSERTMARK";
        g_mapMsgStr[LVM_GETINSERTMARKCOLOR             ] = "LVM_GETINSERTMARKCOLOR";
        g_mapMsgStr[LVM_GETINSERTMARKRECT              ] = "LVM_GETINSERTMARKRECT";
        g_mapMsgStr[LVM_GETISEARCHSTRINGA              ] = "LVM_GETISEARCHSTRINGA";
        g_mapMsgStr[LVM_GETISEARCHSTRINGW              ] = "LVM_GETISEARCHSTRINGW";
        g_mapMsgStr[LVM_GETITEMA                       ] = "LVM_GETITEMA";
        g_mapMsgStr[LVM_GETITEMW                       ] = "LVM_GETITEMW";
        g_mapMsgStr[LVM_GETITEMCOUNT                   ] = "LVM_GETITEMCOUNT";
        g_mapMsgStr[LVM_GETITEMINDEXRECT               ] = "LVM_GETITEMINDEXRECT";
        g_mapMsgStr[LVM_GETITEMPOSITION                ] = "LVM_GETITEMPOSITION";
        g_mapMsgStr[LVM_GETITEMRECT                    ] = "LVM_GETITEMRECT";
        g_mapMsgStr[LVM_GETITEMSPACING                 ] = "LVM_GETITEMSPACING";
        g_mapMsgStr[LVM_GETITEMSTATE                   ] = "LVM_GETITEMSTATE";
        g_mapMsgStr[LVM_GETITEMTEXTA                   ] = "LVM_GETITEMTEXTA";
        g_mapMsgStr[LVM_GETITEMTEXTW                   ] = "LVM_GETITEMTEXTW";
        g_mapMsgStr[LVM_GETNEXTITEM                    ] = "LVM_GETNEXTITEM";
        g_mapMsgStr[LVM_GETNEXTITEMINDEX               ] = "LVM_GETNEXTITEMINDEX";
        g_mapMsgStr[LVM_GETNUMBEROFWORKAREAS           ] = "LVM_GETNUMBEROFWORKAREAS";
        g_mapMsgStr[LVM_GETORIGIN                      ] = "LVM_GETORIGIN";
        g_mapMsgStr[LVM_GETOUTLINECOLOR                ] = "LVM_GETOUTLINECOLOR";
        g_mapMsgStr[LVM_GETSELECTEDCOLUMN              ] = "LVM_GETSELECTEDCOLUMN";
        g_mapMsgStr[LVM_GETSELECTEDCOUNT               ] = "LVM_GETSELECTEDCOUNT";
        g_mapMsgStr[LVM_GETSELECTIONMARK               ] = "LVM_GETSELECTIONMARK";
        g_mapMsgStr[LVM_GETSTRINGWIDTHA                ] = "LVM_GETSTRINGWIDTHA";
        g_mapMsgStr[LVM_GETSTRINGWIDTHW                ] = "LVM_GETSTRINGWIDTHW";
        g_mapMsgStr[LVM_GETSUBITEMRECT                 ] = "LVM_GETSUBITEMRECT";
        g_mapMsgStr[LVM_GETTEXTBKCOLOR                 ] = "LVM_GETTEXTBKCOLOR";
        g_mapMsgStr[LVM_GETTEXTCOLOR                   ] = "LVM_GETTEXTCOLOR";
        g_mapMsgStr[LVM_GETTILEINFO                    ] = "LVM_GETTILEINFO";
        g_mapMsgStr[LVM_GETTILEVIEWINFO                ] = "LVM_GETTILEVIEWINFO";
        g_mapMsgStr[LVM_GETTOOLTIPS                    ] = "LVM_GETTOOLTIPS";
        g_mapMsgStr[LVM_GETTOPINDEX                    ] = "LVM_GETTOPINDEX";
        g_mapMsgStr[LVM_GETVIEW                        ] = "LVM_GETVIEW";
        g_mapMsgStr[LVM_GETVIEWRECT                    ] = "LVM_GETVIEWRECT";
        g_mapMsgStr[LVM_GETWORKAREAS                   ] = "LVM_GETWORKAREAS";
        g_mapMsgStr[LVM_HASGROUP                       ] = "LVM_HASGROUP";
        g_mapMsgStr[LVM_HITTEST                        ] = "LVM_HITTEST";
        g_mapMsgStr[LVM_INSERTCOLUMNA                  ] = "LVM_INSERTCOLUMNA";
        g_mapMsgStr[LVM_INSERTCOLUMNW                  ] = "LVM_INSERTCOLUMNW";
        g_mapMsgStr[LVM_INSERTGROUP                    ] = "LVM_INSERTGROUP";
        g_mapMsgStr[LVM_INSERTGROUPSORTED              ] = "LVM_INSERTGROUPSORTED";
        g_mapMsgStr[LVM_INSERTITEMA                    ] = "LVM_INSERTITEMA";
        g_mapMsgStr[LVM_INSERTITEMW                    ] = "LVM_INSERTITEMW";
        g_mapMsgStr[LVM_INSERTMARKHITTEST              ] = "LVM_INSERTMARKHITTEST";
        g_mapMsgStr[LVM_ISGROUPVIEWENABLED             ] = "LVM_ISGROUPVIEWENABLED";
        g_mapMsgStr[LVM_ISITEMVISIBLE                  ] = "LVM_ISITEMVISIBLE";
        g_mapMsgStr[LVM_MAPIDTOINDEX                   ] = "LVM_MAPIDTOINDEX";
        g_mapMsgStr[LVM_MAPINDEXTOID                   ] = "LVM_MAPINDEXTOID";
        g_mapMsgStr[LVM_MOVEGROUP                      ] = "LVM_MOVEGROUP";
        g_mapMsgStr[LVM_MOVEITEMTOGROUP                ] = "LVM_MOVEITEMTOGROUP";
        g_mapMsgStr[LVM_REDRAWITEMS                    ] = "LVM_REDRAWITEMS";
        g_mapMsgStr[LVM_REMOVEALLGROUPS                ] = "LVM_REMOVEALLGROUPS";
        g_mapMsgStr[LVM_REMOVEGROUP                    ] = "LVM_REMOVEGROUP";
        g_mapMsgStr[LVM_SCROLL                         ] = "LVM_SCROLL";
        g_mapMsgStr[LVM_SETBKCOLOR                     ] = "LVM_SETBKCOLOR";
        g_mapMsgStr[LVM_SETBKIMAGEA                    ] = "LVM_SETBKIMAGEA";
        g_mapMsgStr[LVM_SETBKIMAGEW                    ] = "LVM_SETBKIMAGEW";
        g_mapMsgStr[LVM_SETCALLBACKMASK                ] = "LVM_SETCALLBACKMASK";
        g_mapMsgStr[LVM_SETCOLUMNA                     ] = "LVM_SETCOLUMNA";
        g_mapMsgStr[LVM_SETCOLUMNW                     ] = "LVM_SETCOLUMNW";
        g_mapMsgStr[LVM_SETCOLUMNORDERARRAY            ] = "LVM_SETCOLUMNORDERARRAY";
        g_mapMsgStr[LVM_SETCOLUMNWIDTH                 ] = "LVM_SETCOLUMNWIDTH";
        g_mapMsgStr[LVM_SETEXTENDEDLISTVIEWSTYLE       ] = "LVM_SETEXTENDEDLISTVIEWSTYLE";
        g_mapMsgStr[LVM_SETGROUPINFO                   ] = "LVM_SETGROUPINFO";
        g_mapMsgStr[LVM_SETGROUPMETRICS                ] = "LVM_SETGROUPMETRICS";
        g_mapMsgStr[LVM_SETHOTCURSOR                   ] = "LVM_SETHOTCURSOR";
        g_mapMsgStr[LVM_SETHOTITEM                     ] = "LVM_SETHOTITEM";
        g_mapMsgStr[LVM_SETHOVERTIME                   ] = "LVM_SETHOVERTIME";
        g_mapMsgStr[LVM_SETICONSPACING                 ] = "LVM_SETICONSPACING";
        g_mapMsgStr[LVM_SETIMAGELIST                   ] = "LVM_SETIMAGELIST";
        g_mapMsgStr[LVM_SETINFOTIP                     ] = "LVM_SETINFOTIP";
        g_mapMsgStr[LVM_SETINSERTMARK                  ] = "LVM_SETINSERTMARK";
        g_mapMsgStr[LVM_SETINSERTMARKCOLOR             ] = "LVM_SETINSERTMARKCOLOR";
        g_mapMsgStr[LVM_SETITEMA                       ] = "LVM_SETITEMA";
        g_mapMsgStr[LVM_SETITEMW                       ] = "LVM_SETITEMW";
        g_mapMsgStr[LVM_SETITEMCOUNT                   ] = "LVM_SETITEMCOUNT";
        g_mapMsgStr[LVM_SETITEMINDEXSTATE              ] = "LVM_SETITEMINDEXSTATE";
        g_mapMsgStr[LVM_SETITEMPOSITION                ] = "LVM_SETITEMPOSITION";
        g_mapMsgStr[LVM_SETITEMPOSITION32              ] = "LVM_SETITEMPOSITION32";
        g_mapMsgStr[LVM_SETITEMSTATE                   ] = "LVM_SETITEMSTATE";
        g_mapMsgStr[LVM_SETITEMTEXTA                   ] = "LVM_SETITEMTEXTA";
        g_mapMsgStr[LVM_SETITEMTEXTW                   ] = "LVM_SETITEMTEXTW";
        g_mapMsgStr[LVM_SETOUTLINECOLOR                ] = "LVM_SETOUTLINECOLOR";
        g_mapMsgStr[LVM_SETSELECTEDCOLUMN              ] = "LVM_SETSELECTEDCOLUMN";
        g_mapMsgStr[LVM_SETSELECTIONMARK               ] = "LVM_SETSELECTIONMARK";
        g_mapMsgStr[LVM_SETTEXTBKCOLOR                 ] = "LVM_SETTEXTBKCOLOR";
        g_mapMsgStr[LVM_SETTEXTCOLOR                   ] = "LVM_SETTEXTCOLOR";
        g_mapMsgStr[LVM_SETTILEINFO                    ] = "LVM_SETTILEINFO";
        g_mapMsgStr[LVM_SETTILEVIEWINFO                ] = "LVM_SETTILEVIEWINFO";
        g_mapMsgStr[LVM_SETTOOLTIPS                    ] = "LVM_SETTOOLTIPS";
        g_mapMsgStr[LVM_SETVIEW                        ] = "LVM_SETVIEW";
        g_mapMsgStr[LVM_SETWORKAREAS                   ] = "LVM_SETWORKAREAS";
        g_mapMsgStr[LVM_SORTGROUPS                     ] = "LVM_SORTGROUPS";
        g_mapMsgStr[LVM_SORTITEMS                      ] = "LVM_SORTITEMS";
        g_mapMsgStr[LVM_SORTITEMSEX                    ] = "LVM_SORTITEMSEX";
        g_mapMsgStr[LVM_SUBITEMHITTEST                 ] = "LVM_SUBITEMHITTEST";
        g_mapMsgStr[LVM_UPDATE                         ] = "LVM_UPDATE";
        g_mapMsgStr[MCM_GETCALENDARBORDER              ] = "MCM_GETCALENDARBORDER";
        g_mapMsgStr[MCM_GETCALENDARCOUNT               ] = "MCM_GETCALENDARCOUNT";
        g_mapMsgStr[MCM_GETCALENDARGRIDINFO            ] = "MCM_GETCALENDARGRIDINFO";
        g_mapMsgStr[MCM_GETCALID                       ] = "MCM_GETCALID";
        g_mapMsgStr[MCM_GETCOLOR                       ] = "MCM_GETCOLOR";
        g_mapMsgStr[MCM_GETCURRENTVIEW                 ] = "MCM_GETCURRENTVIEW";
        g_mapMsgStr[MCM_GETCURSEL                      ] = "MCM_GETCURSEL";
        g_mapMsgStr[MCM_GETFIRSTDAYOFWEEK              ] = "MCM_GETFIRSTDAYOFWEEK";
        g_mapMsgStr[MCM_GETMAXSELCOUNT                 ] = "MCM_GETMAXSELCOUNT";
        g_mapMsgStr[MCM_GETMAXTODAYWIDTH               ] = "MCM_GETMAXTODAYWIDTH";
        g_mapMsgStr[MCM_GETMINREQRECT                  ] = "MCM_GETMINREQRECT";
        g_mapMsgStr[MCM_GETMONTHDELTA                  ] = "MCM_GETMONTHDELTA";
        g_mapMsgStr[MCM_GETMONTHRANGE                  ] = "MCM_GETMONTHRANGE";
        g_mapMsgStr[MCM_GETRANGE                       ] = "MCM_GETRANGE";
        g_mapMsgStr[MCM_GETSELRANGE                    ] = "MCM_GETSELRANGE";
        g_mapMsgStr[MCM_GETTODAY                       ] = "MCM_GETTODAY";
        g_mapMsgStr[MCM_HITTEST                        ] = "MCM_HITTEST";
        g_mapMsgStr[MCM_SETCALENDARBORDER              ] = "MCM_SETCALENDARBORDER";
        g_mapMsgStr[MCM_SETCALID                       ] = "MCM_SETCALID";
        g_mapMsgStr[MCM_SETCOLOR                       ] = "MCM_SETCOLOR";
        g_mapMsgStr[MCM_SETCURRENTVIEW                 ] = "MCM_SETCURRENTVIEW";
        g_mapMsgStr[MCM_SETCURSEL                      ] = "MCM_SETCURSEL";
        g_mapMsgStr[MCM_SETDAYSTATE                    ] = "MCM_SETDAYSTATE";
        g_mapMsgStr[MCM_SETFIRSTDAYOFWEEK              ] = "MCM_SETFIRSTDAYOFWEEK";
        g_mapMsgStr[MCM_SETMAXSELCOUNT                 ] = "MCM_SETMAXSELCOUNT";
        g_mapMsgStr[MCM_SETMONTHDELTA                  ] = "MCM_SETMONTHDELTA";
        g_mapMsgStr[MCM_SETRANGE                       ] = "MCM_SETRANGE";
        g_mapMsgStr[MCM_SETSELRANGE                    ] = "MCM_SETSELRANGE";
        g_mapMsgStr[MCM_SETTODAY                       ] = "MCM_SETTODAY";
        g_mapMsgStr[MCM_SIZERECTTOMIN                  ] = "MCM_SIZERECTTOMIN";
        g_mapMsgStr[NCM_DISPLAYERRORTIP                ] = "NCM_DISPLAYERRORTIP";
        g_mapMsgStr[NCM_GETADDRESS                     ] = "NCM_GETADDRESS";
        g_mapMsgStr[NCM_GETALLOWTYPE                   ] = "NCM_GETALLOWTYPE";
        g_mapMsgStr[NCM_SETALLOWTYPE                   ] = "NCM_SETALLOWTYPE";
        g_mapMsgStr[PBM_DELTAPOS                       ] = "PBM_DELTAPOS";
        g_mapMsgStr[PBM_GETBARCOLOR                    ] = "PBM_GETBARCOLOR";
        g_mapMsgStr[PBM_GETBKCOLOR                     ] = "PBM_GETBKCOLOR";
        g_mapMsgStr[PBM_GETPOS                         ] = "PBM_GETPOS";
        g_mapMsgStr[PBM_GETRANGE                       ] = "PBM_GETRANGE";
        g_mapMsgStr[PBM_GETSTATE                       ] = "PBM_GETSTATE";
        g_mapMsgStr[PBM_GETSTEP                        ] = "PBM_GETSTEP";
        g_mapMsgStr[PBM_SETBARCOLOR                    ] = "PBM_SETBARCOLOR";
        g_mapMsgStr[PBM_SETMARQUEE                     ] = "PBM_SETMARQUEE";
        g_mapMsgStr[PBM_SETPOS                         ] = "PBM_SETPOS";
        g_mapMsgStr[PBM_SETRANGE                       ] = "PBM_SETRANGE";
        g_mapMsgStr[PBM_SETRANGE32                     ] = "PBM_SETRANGE32";
        g_mapMsgStr[PBM_SETSTATE                       ] = "PBM_SETSTATE";
        g_mapMsgStr[PBM_SETSTEP                        ] = "PBM_SETSTEP";
        g_mapMsgStr[PBM_STEPIT                         ] = "PBM_STEPIT";
        g_mapMsgStr[PGM_FORWARDMOUSE                   ] = "PGM_FORWARDMOUSE";
        g_mapMsgStr[PGM_GETBKCOLOR                     ] = "PGM_GETBKCOLOR";
        g_mapMsgStr[PGM_GETBORDER                      ] = "PGM_GETBORDER";
        g_mapMsgStr[PGM_GETBUTTONSIZE                  ] = "PGM_GETBUTTONSIZE";
        g_mapMsgStr[PGM_GETBUTTONSTATE                 ] = "PGM_GETBUTTONSTATE";
        g_mapMsgStr[PGM_GETPOS                         ] = "PGM_GETPOS";
        g_mapMsgStr[PGM_RECALCSIZE                     ] = "PGM_RECALCSIZE";
        g_mapMsgStr[PGM_SETBKCOLOR                     ] = "PGM_SETBKCOLOR";
        g_mapMsgStr[PGM_SETBORDER                      ] = "PGM_SETBORDER";
        g_mapMsgStr[PGM_SETBUTTONSIZE                  ] = "PGM_SETBUTTONSIZE";
        g_mapMsgStr[PGM_SETCHILD                       ] = "PGM_SETCHILD";
        g_mapMsgStr[PGM_SETPOS                         ] = "PGM_SETPOS";
        g_mapMsgStr[PGM_SETSCROLLINFO                  ] = "PGM_SETSCROLLINFO";
        g_mapMsgStr[PSM_ADDPAGE                        ] = "PSM_ADDPAGE";
        g_mapMsgStr[PSM_APPLY                          ] = "PSM_APPLY";
        g_mapMsgStr[PSM_CANCELTOCLOSE                  ] = "PSM_CANCELTOCLOSE";
        g_mapMsgStr[PSM_CHANGED                        ] = "PSM_CHANGED";
        g_mapMsgStr[PSM_ENABLEWIZBUTTONS               ] = "PSM_ENABLEWIZBUTTONS";
        g_mapMsgStr[PSM_GETCURRENTPAGEHWND             ] = "PSM_GETCURRENTPAGEHWND";
        g_mapMsgStr[PSM_GETTABCONTROL                  ] = "PSM_GETTABCONTROL";
        g_mapMsgStr[PSM_GETRESULT                      ] = "PSM_GETRESULT";
        g_mapMsgStr[PSM_HWNDTOINDEX                    ] = "PSM_HWNDTOINDEX";
        g_mapMsgStr[PSM_IDTOINDEX                      ] = "PSM_IDTOINDEX";
        g_mapMsgStr[PSM_INDEXTOHWND                    ] = "PSM_INDEXTOHWND";
        g_mapMsgStr[PSM_INDEXTOID                      ] = "PSM_INDEXTOID";
        g_mapMsgStr[PSM_INDEXTOPAGE                    ] = "PSM_INDEXTOPAGE";
        g_mapMsgStr[PSM_INSERTPAGE                     ] = "PSM_INSERTPAGE";
        g_mapMsgStr[PSM_ISDIALOGMESSAGE                ] = "PSM_ISDIALOGMESSAGE";
        g_mapMsgStr[PSM_PAGETOINDEX                    ] = "PSM_PAGETOINDEX";
        g_mapMsgStr[PSM_PRESSBUTTON                    ] = "PSM_PRESSBUTTON";
        g_mapMsgStr[PSM_QUERYSIBLINGS                  ] = "PSM_QUERYSIBLINGS";
        g_mapMsgStr[PSM_REBOOTSYSTEM                   ] = "PSM_REBOOTSYSTEM";
        g_mapMsgStr[PSM_RECALCPAGESIZES                ] = "PSM_RECALCPAGESIZES";
        g_mapMsgStr[PSM_REMOVEPAGE                     ] = "PSM_REMOVEPAGE";
        g_mapMsgStr[PSM_RESTARTWINDOWS                 ] = "PSM_RESTARTWINDOWS";
        g_mapMsgStr[PSM_SETBUTTONTEXTW                 ] = "PSM_SETBUTTONTEXTW";
        g_mapMsgStr[PSM_SETCURSEL                      ] = "PSM_SETCURSEL";
        g_mapMsgStr[PSM_SETCURSELID                    ] = "PSM_SETCURSELID";
        g_mapMsgStr[PSM_SETFINISHTEXTA                 ] = "PSM_SETFINISHTEXTA";
        g_mapMsgStr[PSM_SETFINISHTEXTW                 ] = "PSM_SETFINISHTEXTW";
        g_mapMsgStr[PSM_SETHEADERTITLEA                ] = "PSM_SETHEADERTITLEA";
        g_mapMsgStr[PSM_SETHEADERTITLEW                ] = "PSM_SETHEADERTITLEW";
        g_mapMsgStr[PSM_SETHEADERSUBTITLEA             ] = "PSM_SETHEADERSUBTITLEA";
        g_mapMsgStr[PSM_SETHEADERSUBTITLEW             ] = "PSM_SETHEADERSUBTITLEW";
        g_mapMsgStr[PSM_SETNEXTTEXTW                   ] = "PSM_SETNEXTTEXTW";
        g_mapMsgStr[PSM_SETTITLEA                      ] = "PSM_SETTITLEA";
        g_mapMsgStr[PSM_SETTITLEW                      ] = "PSM_SETTITLEW";
        g_mapMsgStr[PSM_SETWIZBUTTONS                  ] = "PSM_SETWIZBUTTONS";
        g_mapMsgStr[PSM_SHOWWIZBUTTONS                 ] = "PSM_SHOWWIZBUTTONS";
        g_mapMsgStr[PSM_UNCHANGED                      ] = "PSM_UNCHANGED";
        g_mapMsgStr[RB_BEGINDRAG                       ] = "RB_BEGINDRAG";
        g_mapMsgStr[RB_DELETEBAND                      ] = "RB_DELETEBAND";
        g_mapMsgStr[RB_DRAGMOVE                        ] = "RB_DRAGMOVE";
        g_mapMsgStr[RB_ENDDRAG                         ] = "RB_ENDDRAG";
        g_mapMsgStr[RB_GETBANDBORDERS                  ] = "RB_GETBANDBORDERS";
        g_mapMsgStr[RB_GETBANDCOUNT                    ] = "RB_GETBANDCOUNT";
        g_mapMsgStr[RB_GETBANDINFOA                    ] = "RB_GETBANDINFOA";
        g_mapMsgStr[RB_GETBANDINFOW                    ] = "RB_GETBANDINFOW";
        g_mapMsgStr[RB_GETBANDMARGINS                  ] = "RB_GETBANDMARGINS";
        g_mapMsgStr[RB_GETBARHEIGHT                    ] = "RB_GETBARHEIGHT";
        g_mapMsgStr[RB_GETBARINFO                      ] = "RB_GETBARINFO";
        g_mapMsgStr[RB_GETBKCOLOR                      ] = "RB_GETBKCOLOR";
        g_mapMsgStr[RB_GETEXTENDEDSTYLE                ] = "RB_GETEXTENDEDSTYLE";
        g_mapMsgStr[RB_GETPALETTE                      ] = "RB_GETPALETTE";
        g_mapMsgStr[RB_GETRECT                         ] = "RB_GETRECT";
        g_mapMsgStr[RB_GETROWCOUNT                     ] = "RB_GETROWCOUNT";
        g_mapMsgStr[RB_GETROWHEIGHT                    ] = "RB_GETROWHEIGHT";
        g_mapMsgStr[RB_GETTEXTCOLOR                    ] = "RB_GETTEXTCOLOR";
        g_mapMsgStr[RB_GETTOOLTIPS                     ] = "RB_GETTOOLTIPS";
        g_mapMsgStr[RB_HITTEST                         ] = "RB_HITTEST";
        g_mapMsgStr[RB_IDTOINDEX                       ] = "RB_IDTOINDEX";
        g_mapMsgStr[RB_INSERTBANDA                     ] = "RB_INSERTBANDA";
        g_mapMsgStr[RB_INSERTBANDW                     ] = "RB_INSERTBANDW";
        g_mapMsgStr[RB_MAXIMIZEBAND                    ] = "RB_MAXIMIZEBAND";
        g_mapMsgStr[RB_MINIMIZEBAND                    ] = "RB_MINIMIZEBAND";
        g_mapMsgStr[RB_MOVEBAND                        ] = "RB_MOVEBAND";
        g_mapMsgStr[RB_PUSHCHEVRON                     ] = "RB_PUSHCHEVRON";
        g_mapMsgStr[RB_SETBANDINFOA                    ] = "RB_SETBANDINFOA";
        g_mapMsgStr[RB_SETBANDINFOW                    ] = "RB_SETBANDINFOW";
        g_mapMsgStr[RB_SETBANDWIDTH                    ] = "RB_SETBANDWIDTH";
        g_mapMsgStr[RB_SETBARINFO                      ] = "RB_SETBARINFO";
        g_mapMsgStr[RB_SETBKCOLOR                      ] = "RB_SETBKCOLOR";
        g_mapMsgStr[RB_SETEXTENDEDSTYLE                ] = "RB_SETEXTENDEDSTYLE";
        g_mapMsgStr[RB_SETPALETTE                      ] = "RB_SETPALETTE";
        g_mapMsgStr[RB_SETPARENT                       ] = "RB_SETPARENT";
        g_mapMsgStr[RB_SETTEXTCOLOR                    ] = "RB_SETTEXTCOLOR";
        g_mapMsgStr[RB_SETTOOLTIPS                     ] = "RB_SETTOOLTIPS";
        g_mapMsgStr[RB_SHOWBAND                        ] = "RB_SHOWBAND";
        g_mapMsgStr[RB_SIZETORECT                      ] = "RB_SIZETORECT";
        g_mapMsgStr[SB_GETBORDERS                      ] = "SB_GETBORDERS";
        g_mapMsgStr[SB_GETICON                         ] = "SB_GETICON";
        g_mapMsgStr[SB_GETPARTS                        ] = "SB_GETPARTS";
        g_mapMsgStr[SB_GETRECT                         ] = "SB_GETRECT";
        g_mapMsgStr[SB_GETTEXTA                        ] = "SB_GETTEXTA";
        g_mapMsgStr[SB_GETTEXTW                        ] = "SB_GETTEXTW";
        g_mapMsgStr[SB_GETTEXTLENGTHA                  ] = "SB_GETTEXTLENGTHA";
        g_mapMsgStr[SB_GETTEXTLENGTHW                  ] = "SB_GETTEXTLENGTHW";
        g_mapMsgStr[SB_GETTIPTEXTA                     ] = "SB_GETTIPTEXTA";
        g_mapMsgStr[SB_GETTIPTEXTW                     ] = "SB_GETTIPTEXTW";
        g_mapMsgStr[SB_ISSIMPLE                        ] = "SB_ISSIMPLE";
        g_mapMsgStr[SB_SETICON                         ] = "SB_SETICON";
        g_mapMsgStr[SB_SETMINHEIGHT                    ] = "SB_SETMINHEIGHT";
        g_mapMsgStr[SB_SETPARTS                        ] = "SB_SETPARTS";
        g_mapMsgStr[SB_SETTEXTA                        ] = "SB_SETTEXTA";
        g_mapMsgStr[SB_SETTEXTW                        ] = "SB_SETTEXTW";
        g_mapMsgStr[SB_SETTIPTEXTA                     ] = "SB_SETTIPTEXTA";
        g_mapMsgStr[SB_SETTIPTEXTW                     ] = "SB_SETTIPTEXTW";
        g_mapMsgStr[SB_SIMPLE                          ] = "SB_SIMPLE";
        g_mapMsgStr[SBM_ENABLE_ARROWS                  ] = "SBM_ENABLE_ARROWS";
        g_mapMsgStr[SBM_GETPOS                         ] = "SBM_GETPOS";
        g_mapMsgStr[SBM_GETRANGE                       ] = "SBM_GETRANGE";
        g_mapMsgStr[SBM_GETSCROLLBARINFO               ] = "SBM_GETSCROLLBARINFO";
        g_mapMsgStr[SBM_GETSCROLLINFO                  ] = "SBM_GETSCROLLINFO";
        g_mapMsgStr[SBM_SETPOS                         ] = "SBM_SETPOS";
        g_mapMsgStr[SBM_SETRANGE                       ] = "SBM_SETRANGE";
        g_mapMsgStr[SBM_SETRANGEREDRAW                 ] = "SBM_SETRANGEREDRAW";
        g_mapMsgStr[SBM_SETSCROLLINFO                  ] = "SBM_SETSCROLLINFO";
        g_mapMsgStr[STM_GETICON                        ] = "STM_GETICON";
        g_mapMsgStr[STM_GETIMAGE                       ] = "STM_GETIMAGE";
        g_mapMsgStr[STM_SETICON                        ] = "STM_SETICON";
        g_mapMsgStr[STM_SETIMAGE                       ] = "STM_SETIMAGE";
        g_mapMsgStr[TB_ADDBITMAP                       ] = "TB_ADDBITMAP";
        g_mapMsgStr[TB_ADDBUTTONSA                     ] = "TB_ADDBUTTONSA";
        g_mapMsgStr[TB_ADDBUTTONSW                     ] = "TB_ADDBUTTONSW";
        g_mapMsgStr[TB_ADDSTRINGA                      ] = "TB_ADDSTRINGA";
        g_mapMsgStr[TB_ADDSTRINGW                      ] = "TB_ADDSTRINGW";
        g_mapMsgStr[TB_AUTOSIZE                        ] = "TB_AUTOSIZE";
        g_mapMsgStr[TB_BUTTONCOUNT                     ] = "TB_BUTTONCOUNT";
        g_mapMsgStr[TB_BUTTONSTRUCTSIZE                ] = "TB_BUTTONSTRUCTSIZE";
        g_mapMsgStr[TB_CHANGEBITMAP                    ] = "TB_CHANGEBITMAP";
        g_mapMsgStr[TB_CHECKBUTTON                     ] = "TB_CHECKBUTTON";
        g_mapMsgStr[TB_COMMANDTOINDEX                  ] = "TB_COMMANDTOINDEX";
        g_mapMsgStr[TB_CUSTOMIZE                       ] = "TB_CUSTOMIZE";
        g_mapMsgStr[TB_DELETEBUTTON                    ] = "TB_DELETEBUTTON";
        g_mapMsgStr[TB_ENABLEBUTTON                    ] = "TB_ENABLEBUTTON";
        g_mapMsgStr[TB_GETANCHORHIGHLIGHT              ] = "TB_GETANCHORHIGHLIGHT";
        g_mapMsgStr[TB_GETBITMAP                       ] = "TB_GETBITMAP";
        g_mapMsgStr[TB_GETBITMAPFLAGS                  ] = "TB_GETBITMAPFLAGS";
        g_mapMsgStr[TB_GETBUTTON                       ] = "TB_GETBUTTON";
        g_mapMsgStr[TB_GETBUTTONINFOA                  ] = "TB_GETBUTTONINFOA";
        g_mapMsgStr[TB_GETBUTTONINFOW                  ] = "TB_GETBUTTONINFOW";
        g_mapMsgStr[TB_GETBUTTONSIZE                   ] = "TB_GETBUTTONSIZE";
        g_mapMsgStr[TB_GETBUTTONTEXTA                  ] = "TB_GETBUTTONTEXTA";
        g_mapMsgStr[TB_GETBUTTONTEXTW                  ] = "TB_GETBUTTONTEXTW";
        g_mapMsgStr[TB_GETDISABLEDIMAGELIST            ] = "TB_GETDISABLEDIMAGELIST";
        g_mapMsgStr[TB_GETEXTENDEDSTYLE                ] = "TB_GETEXTENDEDSTYLE";
        g_mapMsgStr[TB_GETHOTIMAGELIST                 ] = "TB_GETHOTIMAGELIST";
        g_mapMsgStr[TB_GETHOTITEM                      ] = "TB_GETHOTITEM";
        g_mapMsgStr[TB_GETIDEALSIZE                    ] = "TB_GETIDEALSIZE";
        g_mapMsgStr[TB_GETIMAGELIST                    ] = "TB_GETIMAGELIST";
        g_mapMsgStr[TB_GETIMAGELISTCOUNT               ] = "TB_GETIMAGELISTCOUNT";
        g_mapMsgStr[TB_GETINSERTMARK                   ] = "TB_GETINSERTMARK";
        g_mapMsgStr[TB_GETINSERTMARKCOLOR              ] = "TB_GETINSERTMARKCOLOR";
        g_mapMsgStr[TB_GETITEMDROPDOWNRECT             ] = "TB_GETITEMDROPDOWNRECT";
        g_mapMsgStr[TB_GETITEMRECT                     ] = "TB_GETITEMRECT";
        g_mapMsgStr[TB_GETMAXSIZE                      ] = "TB_GETMAXSIZE";
        g_mapMsgStr[TB_GETMETRICS                      ] = "TB_GETMETRICS";
        g_mapMsgStr[TB_GETOBJECT                       ] = "TB_GETOBJECT";
        g_mapMsgStr[TB_GETPADDING                      ] = "TB_GETPADDING";
        g_mapMsgStr[TB_GETPRESSEDIMAGELIST             ] = "TB_GETPRESSEDIMAGELIST";
        g_mapMsgStr[TB_GETRECT                         ] = "TB_GETRECT";
        g_mapMsgStr[TB_GETROWS                         ] = "TB_GETROWS";
        g_mapMsgStr[TB_GETSTATE                        ] = "TB_GETSTATE";
        g_mapMsgStr[TB_GETSTRINGA                      ] = "TB_GETSTRINGA";
        g_mapMsgStr[TB_GETSTRINGW                      ] = "TB_GETSTRINGW";
        g_mapMsgStr[TB_GETSTYLE                        ] = "TB_GETSTYLE";
        g_mapMsgStr[TB_GETTEXTROWS                     ] = "TB_GETTEXTROWS";
        g_mapMsgStr[TB_GETTOOLTIPS                     ] = "TB_GETTOOLTIPS";
        g_mapMsgStr[TB_HASACCELERATOR                  ] = "TB_HASACCELERATOR";
        g_mapMsgStr[TB_HIDEBUTTON                      ] = "TB_HIDEBUTTON";
        g_mapMsgStr[TB_HITTEST                         ] = "TB_HITTEST";
        g_mapMsgStr[TB_INDETERMINATE                   ] = "TB_INDETERMINATE";
        g_mapMsgStr[TB_INSERTBUTTONA                   ] = "TB_INSERTBUTTONA";
        g_mapMsgStr[TB_INSERTBUTTONW                   ] = "TB_INSERTBUTTONW";
        g_mapMsgStr[TB_INSERTMARKHITTEST               ] = "TB_INSERTMARKHITTEST";
        g_mapMsgStr[TB_ISBUTTONCHECKED                 ] = "TB_ISBUTTONCHECKED";
        g_mapMsgStr[TB_ISBUTTONENABLED                 ] = "TB_ISBUTTONENABLED";
        g_mapMsgStr[TB_ISBUTTONHIDDEN                  ] = "TB_ISBUTTONHIDDEN";
        g_mapMsgStr[TB_ISBUTTONHIGHLIGHTED             ] = "TB_ISBUTTONHIGHLIGHTED";
        g_mapMsgStr[TB_ISBUTTONINDETERMINATE           ] = "TB_ISBUTTONINDETERMINATE";
        g_mapMsgStr[TB_ISBUTTONPRESSED                 ] = "TB_ISBUTTONPRESSED";
        g_mapMsgStr[TB_LOADIMAGES                      ] = "TB_LOADIMAGES";
        g_mapMsgStr[TB_MAPACCELERATORA                 ] = "TB_MAPACCELERATORA";
        g_mapMsgStr[TB_MAPACCELERATORW                 ] = "TB_MAPACCELERATORW";
        g_mapMsgStr[TB_MARKBUTTON                      ] = "TB_MARKBUTTON";
        g_mapMsgStr[TB_MOVEBUTTON                      ] = "TB_MOVEBUTTON";
        g_mapMsgStr[TB_PRESSBUTTON                     ] = "TB_PRESSBUTTON";
        g_mapMsgStr[TB_REPLACEBITMAP                   ] = "TB_REPLACEBITMAP";
        g_mapMsgStr[TB_SAVERESTOREA                    ] = "TB_SAVERESTOREA";
        g_mapMsgStr[TB_SAVERESTOREW                    ] = "TB_SAVERESTOREW";
        g_mapMsgStr[TB_SETANCHORHIGHLIGHT              ] = "TB_SETANCHORHIGHLIGHT";
        g_mapMsgStr[TB_SETBITMAPSIZE                   ] = "TB_SETBITMAPSIZE";
        g_mapMsgStr[TB_SETBOUNDINGSIZE                 ] = "TB_SETBOUNDINGSIZE";
        g_mapMsgStr[TB_SETBUTTONINFOA                  ] = "TB_SETBUTTONINFOA";
        g_mapMsgStr[TB_SETBUTTONINFOW                  ] = "TB_SETBUTTONINFOW";
        g_mapMsgStr[TB_SETBUTTONSIZE                   ] = "TB_SETBUTTONSIZE";
        g_mapMsgStr[TB_SETBUTTONWIDTH                  ] = "TB_SETBUTTONWIDTH";
        g_mapMsgStr[TB_SETCMDID                        ] = "TB_SETCMDID";
        g_mapMsgStr[TB_SETDISABLEDIMAGELIST            ] = "TB_SETDISABLEDIMAGELIST";
        g_mapMsgStr[TB_SETDRAWTEXTFLAGS                ] = "TB_SETDRAWTEXTFLAGS";
        g_mapMsgStr[TB_SETEXTENDEDSTYLE                ] = "TB_SETEXTENDEDSTYLE";
        g_mapMsgStr[TB_SETHOTIMAGELIST                 ] = "TB_SETHOTIMAGELIST";
        g_mapMsgStr[TB_SETHOTITEM                      ] = "TB_SETHOTITEM";
        g_mapMsgStr[TB_SETHOTITEM2                     ] = "TB_SETHOTITEM2";
        g_mapMsgStr[TB_SETIMAGELIST                    ] = "TB_SETIMAGELIST";
        g_mapMsgStr[TB_SETINDENT                       ] = "TB_SETINDENT";
        g_mapMsgStr[TB_SETINSERTMARK                   ] = "TB_SETINSERTMARK";
        g_mapMsgStr[TB_SETINSERTMARKCOLOR              ] = "TB_SETINSERTMARKCOLOR";
        g_mapMsgStr[TB_SETLISTGAP                      ] = "TB_SETLISTGAP";
        g_mapMsgStr[TB_SETMAXTEXTROWS                  ] = "TB_SETMAXTEXTROWS";
        g_mapMsgStr[TB_SETMETRICS                      ] = "TB_SETMETRICS";
        g_mapMsgStr[TB_SETPADDING                      ] = "TB_SETPADDING";
        g_mapMsgStr[TB_SETPARENT                       ] = "TB_SETPARENT";
        g_mapMsgStr[TB_SETPRESSEDIMAGELIST             ] = "TB_SETPRESSEDIMAGELIST";
        g_mapMsgStr[TB_SETROWS                         ] = "TB_SETROWS";
        g_mapMsgStr[TB_SETSTATE                        ] = "TB_SETSTATE";
        g_mapMsgStr[TB_SETSTYLE                        ] = "TB_SETSTYLE";
        g_mapMsgStr[TB_SETTOOLTIPS                     ] = "TB_SETTOOLTIPS";
        g_mapMsgStr[TBM_CLEARSEL                       ] = "TBM_CLEARSEL";
        g_mapMsgStr[TBM_CLEARTICS                      ] = "TBM_CLEARTICS";
        g_mapMsgStr[TBM_GETBUDDY                       ] = "TBM_GETBUDDY";
        g_mapMsgStr[TBM_GETCHANNELRECT                 ] = "TBM_GETCHANNELRECT";
        g_mapMsgStr[TBM_GETLINESIZE                    ] = "TBM_GETLINESIZE";
        g_mapMsgStr[TBM_GETNUMTICS                     ] = "TBM_GETNUMTICS";
        g_mapMsgStr[TBM_GETPAGESIZE                    ] = "TBM_GETPAGESIZE";
        g_mapMsgStr[TBM_GETPOS                         ] = "TBM_GETPOS";
        g_mapMsgStr[TBM_GETPTICS                       ] = "TBM_GETPTICS";
        g_mapMsgStr[TBM_GETRANGEMAX                    ] = "TBM_GETRANGEMAX";
        g_mapMsgStr[TBM_GETRANGEMIN                    ] = "TBM_GETRANGEMIN";
        g_mapMsgStr[TBM_GETSELEND                      ] = "TBM_GETSELEND";
        g_mapMsgStr[TBM_GETSELSTART                    ] = "TBM_GETSELSTART";
        g_mapMsgStr[TBM_GETTHUMBLENGTH                 ] = "TBM_GETTHUMBLENGTH";
        g_mapMsgStr[TBM_GETTHUMBRECT                   ] = "TBM_GETTHUMBRECT";
        g_mapMsgStr[TBM_GETTIC                         ] = "TBM_GETTIC";
        g_mapMsgStr[TBM_GETTICPOS                      ] = "TBM_GETTICPOS";
        g_mapMsgStr[TBM_GETTOOLTIPS                    ] = "TBM_GETTOOLTIPS";
        g_mapMsgStr[TBM_SETBUDDY                       ] = "TBM_SETBUDDY";
        g_mapMsgStr[TBM_SETLINESIZE                    ] = "TBM_SETLINESIZE";
        g_mapMsgStr[TBM_SETPAGESIZE                    ] = "TBM_SETPAGESIZE";
        g_mapMsgStr[TBM_SETPOS                         ] = "TBM_SETPOS";
        g_mapMsgStr[TBM_SETPOSNOTIFY                   ] = "TBM_SETPOSNOTIFY";
        g_mapMsgStr[TBM_SETRANGE                       ] = "TBM_SETRANGE";
        g_mapMsgStr[TBM_SETRANGEMAX                    ] = "TBM_SETRANGEMAX";
        g_mapMsgStr[TBM_SETRANGEMIN                    ] = "TBM_SETRANGEMIN";
        g_mapMsgStr[TBM_SETSEL                         ] = "TBM_SETSEL";
        g_mapMsgStr[TBM_SETSELEND                      ] = "TBM_SETSELEND";
        g_mapMsgStr[TBM_SETSELSTART                    ] = "TBM_SETSELSTART";
        g_mapMsgStr[TBM_SETTHUMBLENGTH                 ] = "TBM_SETTHUMBLENGTH";
        g_mapMsgStr[TBM_SETTIC                         ] = "TBM_SETTIC";
        g_mapMsgStr[TBM_SETTICFREQ                     ] = "TBM_SETTICFREQ";
        g_mapMsgStr[TBM_SETTIPSIDE                     ] = "TBM_SETTIPSIDE";
        g_mapMsgStr[TBM_SETTOOLTIPS                    ] = "TBM_SETTOOLTIPS";
        g_mapMsgStr[TCM_ADJUSTRECT                     ] = "TCM_ADJUSTRECT";
        g_mapMsgStr[TCM_DELETEALLITEMS                 ] = "TCM_DELETEALLITEMS";
        g_mapMsgStr[TCM_DELETEITEM                     ] = "TCM_DELETEITEM";
        g_mapMsgStr[TCM_DESELECTALL                    ] = "TCM_DESELECTALL";
        g_mapMsgStr[TCM_GETCURFOCUS                    ] = "TCM_GETCURFOCUS";
        g_mapMsgStr[TCM_GETCURSEL                      ] = "TCM_GETCURSEL";
        g_mapMsgStr[TCM_GETEXTENDEDSTYLE               ] = "TCM_GETEXTENDEDSTYLE";
        g_mapMsgStr[TCM_GETIMAGELIST                   ] = "TCM_GETIMAGELIST";
        g_mapMsgStr[TCM_GETITEMA                       ] = "TCM_GETITEMA";
        g_mapMsgStr[TCM_GETITEMW                       ] = "TCM_GETITEMW";
        g_mapMsgStr[TCM_GETITEMCOUNT                   ] = "TCM_GETITEMCOUNT";
        g_mapMsgStr[TCM_GETITEMRECT                    ] = "TCM_GETITEMRECT";
        g_mapMsgStr[TCM_GETROWCOUNT                    ] = "TCM_GETROWCOUNT";
        g_mapMsgStr[TCM_GETTOOLTIPS                    ] = "TCM_GETTOOLTIPS";
        g_mapMsgStr[TCM_HIGHLIGHTITEM                  ] = "TCM_HIGHLIGHTITEM";
        g_mapMsgStr[TCM_HITTEST                        ] = "TCM_HITTEST";
        g_mapMsgStr[TCM_INSERTITEMA                    ] = "TCM_INSERTITEMA";
        g_mapMsgStr[TCM_INSERTITEMW                    ] = "TCM_INSERTITEMW";
        g_mapMsgStr[TCM_REMOVEIMAGE                    ] = "TCM_REMOVEIMAGE";
        g_mapMsgStr[TCM_SETCURFOCUS                    ] = "TCM_SETCURFOCUS";
        g_mapMsgStr[TCM_SETCURSEL                      ] = "TCM_SETCURSEL";
        g_mapMsgStr[TCM_SETEXTENDEDSTYLE               ] = "TCM_SETEXTENDEDSTYLE";
        g_mapMsgStr[TCM_SETIMAGELIST                   ] = "TCM_SETIMAGELIST";
        g_mapMsgStr[TCM_SETITEMA                       ] = "TCM_SETITEMA";
        g_mapMsgStr[TCM_SETITEMW                       ] = "TCM_SETITEMW";
        g_mapMsgStr[TCM_SETITEMEXTRA                   ] = "TCM_SETITEMEXTRA";
        g_mapMsgStr[TCM_SETITEMSIZE                    ] = "TCM_SETITEMSIZE";
        g_mapMsgStr[TCM_SETMINTABWIDTH                 ] = "TCM_SETMINTABWIDTH";
        g_mapMsgStr[TCM_SETPADDING                     ] = "TCM_SETPADDING";
        g_mapMsgStr[TCM_SETTOOLTIPS                    ] = "TCM_SETTOOLTIPS";
        g_mapMsgStr[TTM_ACTIVATE                       ] = "TTM_ACTIVATE";
        g_mapMsgStr[TTM_ADDTOOLA                       ] = "TTM_ADDTOOLA";
        g_mapMsgStr[TTM_ADDTOOLW                       ] = "TTM_ADDTOOLW";
        g_mapMsgStr[TTM_ADJUSTRECT                     ] = "TTM_ADJUSTRECT";
        g_mapMsgStr[TTM_DELTOOLA                       ] = "TTM_DELTOOLA";
        g_mapMsgStr[TTM_DELTOOLW                       ] = "TTM_DELTOOLW";
        g_mapMsgStr[TTM_ENUMTOOLSA                     ] = "TTM_ENUMTOOLSA";
        g_mapMsgStr[TTM_ENUMTOOLSW                     ] = "TTM_ENUMTOOLSW";
        g_mapMsgStr[TTM_GETBUBBLESIZE                  ] = "TTM_GETBUBBLESIZE";
        g_mapMsgStr[TTM_GETCURRENTTOOLA                ] = "TTM_GETCURRENTTOOLA";
        g_mapMsgStr[TTM_GETCURRENTTOOLW                ] = "TTM_GETCURRENTTOOLW";
        g_mapMsgStr[TTM_GETDELAYTIME                   ] = "TTM_GETDELAYTIME";
        g_mapMsgStr[TTM_GETMARGIN                      ] = "TTM_GETMARGIN";
        g_mapMsgStr[TTM_GETMAXTIPWIDTH                 ] = "TTM_GETMAXTIPWIDTH";
        g_mapMsgStr[TTM_GETTEXTA                       ] = "TTM_GETTEXTA";
        g_mapMsgStr[TTM_GETTEXTW                       ] = "TTM_GETTEXTW";
        g_mapMsgStr[TTM_GETTIPBKCOLOR                  ] = "TTM_GETTIPBKCOLOR";
        g_mapMsgStr[TTM_GETTIPTEXTCOLOR                ] = "TTM_GETTIPTEXTCOLOR";
        g_mapMsgStr[TTM_GETTITLE                       ] = "TTM_GETTITLE";
        g_mapMsgStr[TTM_GETTOOLCOUNT                   ] = "TTM_GETTOOLCOUNT";
        g_mapMsgStr[TTM_GETTOOLINFOA                   ] = "TTM_GETTOOLINFOA";
        g_mapMsgStr[TTM_GETTOOLINFOW                   ] = "TTM_GETTOOLINFOW";
        g_mapMsgStr[TTM_HITTESTA                       ] = "TTM_HITTESTA";
        g_mapMsgStr[TTM_HITTESTW                       ] = "TTM_HITTESTW";
        g_mapMsgStr[TTM_NEWTOOLRECTA                   ] = "TTM_NEWTOOLRECTA";
        g_mapMsgStr[TTM_NEWTOOLRECTW                   ] = "TTM_NEWTOOLRECTW";
        g_mapMsgStr[TTM_POP                            ] = "TTM_POP";
        g_mapMsgStr[TTM_POPUP                          ] = "TTM_POPUP";
        g_mapMsgStr[TTM_RELAYEVENT                     ] = "TTM_RELAYEVENT";
        g_mapMsgStr[TTM_SETDELAYTIME                   ] = "TTM_SETDELAYTIME";
        g_mapMsgStr[TTM_SETMARGIN                      ] = "TTM_SETMARGIN";
        g_mapMsgStr[TTM_SETMAXTIPWIDTH                 ] = "TTM_SETMAXTIPWIDTH";
        g_mapMsgStr[TTM_SETTIPBKCOLOR                  ] = "TTM_SETTIPBKCOLOR";
        g_mapMsgStr[TTM_SETTIPTEXTCOLOR                ] = "TTM_SETTIPTEXTCOLOR";
        g_mapMsgStr[TTM_SETTITLEA                      ] = "TTM_SETTITLEA";
        g_mapMsgStr[TTM_SETTITLEW                      ] = "TTM_SETTITLEW";
        g_mapMsgStr[TTM_SETTOOLINFOA                   ] = "TTM_SETTOOLINFOA";
        g_mapMsgStr[TTM_SETTOOLINFOW                   ] = "TTM_SETTOOLINFOW";
        g_mapMsgStr[TTM_TRACKACTIVATE                  ] = "TTM_TRACKACTIVATE";
        g_mapMsgStr[TTM_TRACKPOSITION                  ] = "TTM_TRACKPOSITION";
        g_mapMsgStr[TTM_UPDATE                         ] = "TTM_UPDATE";
        g_mapMsgStr[TTM_UPDATETIPTEXTA                 ] = "TTM_UPDATETIPTEXTA";
        g_mapMsgStr[TTM_UPDATETIPTEXTW                 ] = "TTM_UPDATETIPTEXTW";
        g_mapMsgStr[TTM_WINDOWFROMPOINT                ] = "TTM_WINDOWFROMPOINT";
        g_mapMsgStr[TVM_CREATEDRAGIMAGE                ] = "TVM_CREATEDRAGIMAGE";
        g_mapMsgStr[TVM_DELETEITEM                     ] = "TVM_DELETEITEM";
        g_mapMsgStr[TVM_EDITLABELA                     ] = "TVM_EDITLABELA";
        g_mapMsgStr[TVM_EDITLABELW                     ] = "TVM_EDITLABELW";
        g_mapMsgStr[TVM_ENDEDITLABELNOW                ] = "TVM_ENDEDITLABELNOW";
        g_mapMsgStr[TVM_ENSUREVISIBLE                  ] = "TVM_ENSUREVISIBLE";
        g_mapMsgStr[TVM_EXPAND                         ] = "TVM_EXPAND";
        g_mapMsgStr[TVM_GETBKCOLOR                     ] = "TVM_GETBKCOLOR";
        g_mapMsgStr[TVM_GETCOUNT                       ] = "TVM_GETCOUNT";
        g_mapMsgStr[TVM_GETEDITCONTROL                 ] = "TVM_GETEDITCONTROL";
        g_mapMsgStr[TVM_GETEXTENDEDSTYLE               ] = "TVM_GETEXTENDEDSTYLE";
        g_mapMsgStr[TVM_GETIMAGELIST                   ] = "TVM_GETIMAGELIST";
        g_mapMsgStr[TVM_GETINDENT                      ] = "TVM_GETINDENT";
        g_mapMsgStr[TVM_GETINSERTMARKCOLOR             ] = "TVM_GETINSERTMARKCOLOR";
        g_mapMsgStr[TVM_GETISEARCHSTRINGA              ] = "TVM_GETISEARCHSTRINGA";
        g_mapMsgStr[TVM_GETISEARCHSTRINGW              ] = "TVM_GETISEARCHSTRINGW";
        g_mapMsgStr[TVM_GETITEMA                       ] = "TVM_GETITEMA";
        g_mapMsgStr[TVM_GETITEMW                       ] = "TVM_GETITEMW";
        g_mapMsgStr[TVM_GETITEMHEIGHT                  ] = "TVM_GETITEMHEIGHT";
        g_mapMsgStr[TVM_GETITEMPARTRECT                ] = "TVM_GETITEMPARTRECT";
        g_mapMsgStr[TVM_GETITEMRECT                    ] = "TVM_GETITEMRECT";
        g_mapMsgStr[TVM_GETITEMSTATE                   ] = "TVM_GETITEMSTATE";
        g_mapMsgStr[TVM_GETLINECOLOR                   ] = "TVM_GETLINECOLOR";
        g_mapMsgStr[TVM_GETNEXTITEM                    ] = "TVM_GETNEXTITEM";
        g_mapMsgStr[TVM_GETSCROLLTIME                  ] = "TVM_GETSCROLLTIME";
        g_mapMsgStr[TVM_GETSELECTEDCOUNT               ] = "TVM_GETSELECTEDCOUNT";
        g_mapMsgStr[TVM_GETTEXTCOLOR                   ] = "TVM_GETTEXTCOLOR";
        g_mapMsgStr[TVM_GETTOOLTIPS                    ] = "TVM_GETTOOLTIPS";
        g_mapMsgStr[TVM_GETVISIBLECOUNT                ] = "TVM_GETVISIBLECOUNT";
        g_mapMsgStr[TVM_HITTEST                        ] = "TVM_HITTEST";
        g_mapMsgStr[TVM_INSERTITEMA                    ] = "TVM_INSERTITEMA";
        g_mapMsgStr[TVM_INSERTITEMW                    ] = "TVM_INSERTITEMW";
        g_mapMsgStr[TVM_MAPACCIDTOHTREEITEM            ] = "TVM_MAPACCIDTOHTREEITEM";
        g_mapMsgStr[TVM_MAPHTREEITEMTOACCID            ] = "TVM_MAPHTREEITEMTOACCID";
        g_mapMsgStr[TVM_SELECTITEM                     ] = "TVM_SELECTITEM";
        g_mapMsgStr[TVM_SETAUTOSCROLLINFO              ] = "TVM_SETAUTOSCROLLINFO";
        g_mapMsgStr[TVM_SETBKCOLOR                     ] = "TVM_SETBKCOLOR";
        g_mapMsgStr[TVM_SETBORDER                      ] = "TVM_SETBORDER";
        g_mapMsgStr[TVM_SETEXTENDEDSTYLE               ] = "TVM_SETEXTENDEDSTYLE";
        g_mapMsgStr[TVM_SETHOT                         ] = "TVM_SETHOT";
        g_mapMsgStr[TVM_SETIMAGELIST                   ] = "TVM_SETIMAGELIST";
        g_mapMsgStr[TVM_SETINDENT                      ] = "TVM_SETINDENT";
        g_mapMsgStr[TVM_SETINSERTMARK                  ] = "TVM_SETINSERTMARK";
        g_mapMsgStr[TVM_SETINSERTMARKCOLOR             ] = "TVM_SETINSERTMARKCOLOR";
        g_mapMsgStr[TVM_SETITEMA                       ] = "TVM_SETITEMA";
        g_mapMsgStr[TVM_SETITEMW                       ] = "TVM_SETITEMW";
        g_mapMsgStr[TVM_SETITEMHEIGHT                  ] = "TVM_SETITEMHEIGHT";
        g_mapMsgStr[TVM_SETLINECOLOR                   ] = "TVM_SETLINECOLOR";
        g_mapMsgStr[TVM_SETSCROLLTIME                  ] = "TVM_SETSCROLLTIME";
        g_mapMsgStr[TVM_SETTEXTCOLOR                   ] = "TVM_SETTEXTCOLOR";
        g_mapMsgStr[TVM_SETTOOLTIPS                    ] = "TVM_SETTOOLTIPS";
        g_mapMsgStr[TVM_SHOWINFOTIP                    ] = "TVM_SHOWINFOTIP";
        g_mapMsgStr[TVM_SORTCHILDREN                   ] = "TVM_SORTCHILDREN";
        g_mapMsgStr[TVM_SORTCHILDRENCB                 ] = "TVM_SORTCHILDRENCB";
        g_mapMsgStr[UDM_GETACCEL                       ] = "UDM_GETACCEL";
        g_mapMsgStr[UDM_GETBASE                        ] = "UDM_GETBASE";
        g_mapMsgStr[UDM_GETBUDDY                       ] = "UDM_GETBUDDY";
        g_mapMsgStr[UDM_GETPOS                         ] = "UDM_GETPOS";
        g_mapMsgStr[UDM_GETPOS32                       ] = "UDM_GETPOS32";
        g_mapMsgStr[UDM_GETRANGE                       ] = "UDM_GETRANGE";
        g_mapMsgStr[UDM_GETRANGE32                     ] = "UDM_GETRANGE32";
        g_mapMsgStr[UDM_SETACCEL                       ] = "UDM_SETACCEL";
        g_mapMsgStr[UDM_SETBASE                        ] = "UDM_SETBASE";
        g_mapMsgStr[UDM_SETBUDDY                       ] = "UDM_SETBUDDY";
        g_mapMsgStr[UDM_SETPOS                         ] = "UDM_SETPOS";
        g_mapMsgStr[UDM_SETPOS32                       ] = "UDM_SETPOS32";
        g_mapMsgStr[UDM_SETRANGE                       ] = "UDM_SETRANGE";
        g_mapMsgStr[UDM_SETRANGE32                     ] = "UDM_SETRANGE32";
        g_mapMsgStr[WM_ACTIVATE                        ] = "WM_ACTIVATE";
        g_mapMsgStr[WM_ACTIVATEAPP                     ] = "WM_ACTIVATEAPP";
        g_mapMsgStr[WM_ACTIVATETOPLEVEL                ] = "WM_ACTIVATETOPLEVEL";
        g_mapMsgStr[WM_APPCOMMAND                      ] = "WM_APPCOMMAND";
        g_mapMsgStr[WM_ASKCBFORMATNAME                 ] = "WM_ASKCBFORMATNAME";
        g_mapMsgStr[WM_CANCELJOURNAL                   ] = "WM_CANCELJOURNAL";
        g_mapMsgStr[WM_CANCELMODE                      ] = "WM_CANCELMODE";
        g_mapMsgStr[WM_CAPTURECHANGED                  ] = "WM_CAPTURECHANGED";
        g_mapMsgStr[WM_CHANGECBCHAIN                   ] = "WM_CHANGECBCHAIN";
        g_mapMsgStr[WM_CHANGEUISTATE                   ] = "WM_CHANGEUISTATE";
        g_mapMsgStr[WM_CHAR                            ] = "WM_CHAR";
        g_mapMsgStr[WM_CHARTOITEM                      ] = "WM_CHARTOITEM";
        g_mapMsgStr[WM_CHILDACTIVATE                   ] = "WM_CHILDACTIVATE";
        g_mapMsgStr[WM_CHOOSEFONT_GETLOGFONT           ] = "WM_CHOOSEFONT_GETLOGFONT";
        g_mapMsgStr[WM_CHOOSEFONT_SETFLAGS             ] = "WM_CHOOSEFONT_SETFLAGS";
        g_mapMsgStr[WM_CHOOSEFONT_SETLOGFONT           ] = "WM_CHOOSEFONT_SETLOGFONT";
        g_mapMsgStr[WM_CLEAR                           ] = "WM_CLEAR";
        g_mapMsgStr[WM_CLIPBOARDUPDATE                 ] = "WM_CLIPBOARDUPDATE";
        g_mapMsgStr[WM_CLOSE                           ] = "WM_CLOSE";
        g_mapMsgStr[WM_COMMAND                         ] = "WM_COMMAND";
        g_mapMsgStr[WM_COMMANDHELP                     ] = "WM_COMMANDHELP";
        g_mapMsgStr[WM_COMPACTING                      ] = "WM_COMPACTING";
        g_mapMsgStr[WM_COMPAREITEM                     ] = "WM_COMPAREITEM";
        g_mapMsgStr[WM_CONTEXTMENU                     ] = "WM_CONTEXTMENU";
        g_mapMsgStr[WM_WNT_CONVERTREQUESTEX            ] = "WM_WNT_CONVERTREQUESTEX";
        g_mapMsgStr[WM_COPY                            ] = "WM_COPY";
        g_mapMsgStr[WM_COPYDATA                        ] = "WM_COPYDATA";
        g_mapMsgStr[WM_CREATE                          ] = "WM_CREATE";
        g_mapMsgStr[WM_CTLCOLORBTN                     ] = "WM_CTLCOLORBTN";
        g_mapMsgStr[WM_CTLCOLORDLG                     ] = "WM_CTLCOLORDLG";
        g_mapMsgStr[WM_CTLCOLOREDIT                    ] = "WM_CTLCOLOREDIT";
        g_mapMsgStr[WM_CTLCOLORLISTBOX                 ] = "WM_CTLCOLORLISTBOX";
        g_mapMsgStr[WM_CTLCOLORMSGBOX                  ] = "WM_CTLCOLORMSGBOX";
        g_mapMsgStr[WM_CTLCOLORSCROLLBAR               ] = "WM_CTLCOLORSCROLLBAR";
        g_mapMsgStr[WM_CTLCOLORSTATIC                  ] = "WM_CTLCOLORSTATIC";
        g_mapMsgStr[WM_CUT                             ] = "WM_CUT";
        g_mapMsgStr[WM_DDE_ACK                         ] = "WM_DDE_ACK";
        g_mapMsgStr[WM_DDE_ADVISE                      ] = "WM_DDE_ADVISE";
        g_mapMsgStr[WM_DDE_DATA                        ] = "WM_DDE_DATA";
        g_mapMsgStr[WM_DDE_EXECUTE                     ] = "WM_DDE_EXECUTE";
        g_mapMsgStr[WM_DDE_INITIATE                    ] = "WM_DDE_INITIATE";
        g_mapMsgStr[WM_DDE_POKE                        ] = "WM_DDE_POKE";
        g_mapMsgStr[WM_DDE_REQUEST                     ] = "WM_DDE_REQUEST";
        g_mapMsgStr[WM_DDE_TERMINATE                   ] = "WM_DDE_TERMINATE";
        g_mapMsgStr[WM_DDE_UNADVISE                    ] = "WM_DDE_UNADVISE";
        g_mapMsgStr[WM_DEADCHAR                        ] = "WM_DEADCHAR";
        g_mapMsgStr[WM_DELETEITEM                      ] = "WM_DELETEITEM";
        g_mapMsgStr[WM_DESTROY                         ] = "WM_DESTROY";
        g_mapMsgStr[WM_DESTROYCLIPBOARD                ] = "WM_DESTROYCLIPBOARD";
        g_mapMsgStr[WM_DEVICECHANGE                    ] = "WM_DEVICECHANGE";
        g_mapMsgStr[WM_DEVMODECHANGE                   ] = "WM_DEVMODECHANGE";
        g_mapMsgStr[WM_DISABLEMODAL                    ] = "WM_DISABLEMODAL";
        g_mapMsgStr[WM_DISPLAYCHANGE                   ] = "WM_DISPLAYCHANGE";
        g_mapMsgStr[WM_DRAWCLIPBOARD                   ] = "WM_DRAWCLIPBOARD";
        g_mapMsgStr[WM_DRAWITEM                        ] = "WM_DRAWITEM";
        g_mapMsgStr[WM_DROPFILES                       ] = "WM_DROPFILES";
        g_mapMsgStr[WM_DWMCOLORIZATIONCOLORCHANGED     ] = "WM_DWMCOLORIZATIONCOLORCHANGED";
        g_mapMsgStr[WM_DWMCOMPOSITIONCHANGED           ] = "WM_DWMCOMPOSITIONCHANGED";
        g_mapMsgStr[WM_DWMNCRENDERINGCHANGED           ] = "WM_DWMNCRENDERINGCHANGED";
        g_mapMsgStr[WM_DWMSENDICONICLIVEPREVIEWBITMAP  ] = "WM_DWMSENDICONICLIVEPREVIEWBITMAP";
        g_mapMsgStr[WM_DWMSENDICONICTHUMBNAIL          ] = "WM_DWMSENDICONICTHUMBNAIL";
        g_mapMsgStr[WM_DWMWINDOWMAXIMIZEDCHANGE        ] = "WM_DWMWINDOWMAXIMIZEDCHANGE";
        g_mapMsgStr[WM_ENABLE                          ] = "WM_ENABLE";
        g_mapMsgStr[WM_ENDSESSION                      ] = "WM_ENDSESSION";
        g_mapMsgStr[WM_ENTERIDLE                       ] = "WM_ENTERIDLE";
        g_mapMsgStr[WM_ENTERMENULOOP                   ] = "WM_ENTERMENULOOP";
        g_mapMsgStr[WM_ENTERSIZEMOVE                   ] = "WM_ENTERSIZEMOVE";
        g_mapMsgStr[WM_ERASEBKGND                      ] = "WM_ERASEBKGND";
        g_mapMsgStr[WM_EXITHELPMODE                    ] = "WM_EXITHELPMODE";
        g_mapMsgStr[WM_EXITMENULOOP                    ] = "WM_EXITMENULOOP";
        g_mapMsgStr[WM_EXITSIZEMOVE                    ] = "WM_EXITSIZEMOVE";
        g_mapMsgStr[WM_FLOATSTATUS                     ] = "WM_FLOATSTATUS";
        g_mapMsgStr[WM_FONTCHANGE                      ] = "WM_FONTCHANGE";
        g_mapMsgStr[WM_GESTURE                         ] = "WM_GESTURE";
        g_mapMsgStr[WM_GESTURENOTIFY                   ] = "WM_GESTURENOTIFY";
        g_mapMsgStr[WM_GETDLGCODE                      ] = "WM_GETDLGCODE";
        g_mapMsgStr[WM_GETFONT                         ] = "WM_GETFONT";
        g_mapMsgStr[WM_GETHOTKEY                       ] = "WM_GETHOTKEY";
        g_mapMsgStr[WM_GETICON                         ] = "WM_GETICON";
        g_mapMsgStr[WM_GETMINMAXINFO                   ] = "WM_GETMINMAXINFO";
        g_mapMsgStr[WM_GETOBJECT                       ] = "WM_GETOBJECT";
        g_mapMsgStr[WM_GETTEXT                         ] = "WM_GETTEXT";
        g_mapMsgStr[WM_GETTEXTLENGTH                   ] = "WM_GETTEXTLENGTH";
        g_mapMsgStr[WM_GETTITLEBARINFOEX               ] = "WM_GETTITLEBARINFOEX";
        g_mapMsgStr[WM_HELP                            ] = "WM_HELP";
        g_mapMsgStr[WM_HELPHITTEST                     ] = "WM_HELPHITTEST";
        g_mapMsgStr[WM_HOTKEY                          ] = "WM_HOTKEY";
        g_mapMsgStr[WM_HSCROLL                         ] = "WM_HSCROLL";
        g_mapMsgStr[WM_HSCROLLCLIPBOARD                ] = "WM_HSCROLLCLIPBOARD";
        g_mapMsgStr[WM_ICONERASEBKGND                  ] = "WM_ICONERASEBKGND";
        g_mapMsgStr[WM_IDLEUPDATECMDUI                 ] = "WM_IDLEUPDATECMDUI";
        g_mapMsgStr[WM_IME_CHAR                        ] = "WM_IME_CHAR";
        g_mapMsgStr[WM_IME_COMPOSITION                 ] = "WM_IME_COMPOSITION";
        g_mapMsgStr[WM_IME_COMPOSITIONFULL             ] = "WM_IME_COMPOSITIONFULL";
        g_mapMsgStr[WM_IME_CONTROL                     ] = "WM_IME_CONTROL";
        g_mapMsgStr[WM_IME_ENDCOMPOSITION              ] = "WM_IME_ENDCOMPOSITION";
        g_mapMsgStr[WM_IME_KEYDOWN                     ] = "WM_IME_KEYDOWN";
        g_mapMsgStr[WM_IME_KEYUP                       ] = "WM_IME_KEYUP";
        g_mapMsgStr[WM_IME_NOTIFY                      ] = "WM_IME_NOTIFY";
        g_mapMsgStr[WM_IME_REQUEST                     ] = "WM_IME_REQUEST";
        g_mapMsgStr[WM_IME_SELECT                      ] = "WM_IME_SELECT";
        g_mapMsgStr[WM_IME_SETCONTEXT                  ] = "WM_IME_SETCONTEXT";
        g_mapMsgStr[WM_IME_STARTCOMPOSITION            ] = "WM_IME_STARTCOMPOSITION";
        g_mapMsgStr[WM_INITDIALOG                      ] = "WM_INITDIALOG";
        g_mapMsgStr[WM_INITIALUPDATE                   ] = "WM_INITIALUPDATE";
        g_mapMsgStr[WM_INITMENU                        ] = "WM_INITMENU";
        g_mapMsgStr[WM_INITMENUPOPUP                   ] = "WM_INITMENUPOPUP";
        g_mapMsgStr[WM_INPUT                           ] = "WM_INPUT";
        g_mapMsgStr[WM_INPUT_DEVICE_CHANGE             ] = "WM_INPUT_DEVICE_CHANGE";
        g_mapMsgStr[WM_INPUTLANGCHANGE                 ] = "WM_INPUTLANGCHANGE";
        g_mapMsgStr[WM_INPUTLANGCHANGEREQUEST          ] = "WM_INPUTLANGCHANGEREQUEST";
        g_mapMsgStr[WM_KEYDOWN                         ] = "WM_KEYDOWN";
        g_mapMsgStr[WM_KEYUP                           ] = "WM_KEYUP";
        g_mapMsgStr[WM_KICKIDLE                        ] = "WM_KICKIDLE";
        g_mapMsgStr[WM_KILLFOCUS                       ] = "WM_KILLFOCUS";
        g_mapMsgStr[WM_LBUTTONDBLCLK                   ] = "WM_LBUTTONDBLCLK";
        g_mapMsgStr[WM_LBUTTONDOWN                     ] = "WM_LBUTTONDOWN";
        g_mapMsgStr[WM_LBUTTONUP                       ] = "WM_LBUTTONUP";
        g_mapMsgStr[WM_MBUTTONDBLCLK                   ] = "WM_MBUTTONDBLCLK";
        g_mapMsgStr[WM_MBUTTONDOWN                     ] = "WM_MBUTTONDOWN";
        g_mapMsgStr[WM_MBUTTONUP                       ] = "WM_MBUTTONUP";
        g_mapMsgStr[WM_MDIACTIVATE                     ] = "WM_MDIACTIVATE";
        g_mapMsgStr[WM_MDICASCADE                      ] = "WM_MDICASCADE";
        g_mapMsgStr[WM_MDICREATE                       ] = "WM_MDICREATE";
        g_mapMsgStr[WM_MDIDESTROY                      ] = "WM_MDIDESTROY";
        g_mapMsgStr[WM_MDIGETACTIVE                    ] = "WM_MDIGETACTIVE";
        g_mapMsgStr[WM_MDIICONARRANGE                  ] = "WM_MDIICONARRANGE";
        g_mapMsgStr[WM_MDIMAXIMIZE                     ] = "WM_MDIMAXIMIZE";
        g_mapMsgStr[WM_MDINEXT                         ] = "WM_MDINEXT";
        g_mapMsgStr[WM_MDIREFRESHMENU                  ] = "WM_MDIREFRESHMENU";
        g_mapMsgStr[WM_MDIRESTORE                      ] = "WM_MDIRESTORE";
        g_mapMsgStr[WM_MDISETMENU                      ] = "WM_MDISETMENU";
        g_mapMsgStr[WM_MDITILE                         ] = "WM_MDITILE";
        g_mapMsgStr[WM_MEASUREITEM                     ] = "WM_MEASUREITEM";
        g_mapMsgStr[WM_MENUCHAR                        ] = "WM_MENUCHAR";
        g_mapMsgStr[WM_MENUCOMMAND                     ] = "WM_MENUCOMMAND";
        g_mapMsgStr[WM_MENUDRAG                        ] = "WM_MENUDRAG";
        g_mapMsgStr[WM_MENUGETOBJECT                   ] = "WM_MENUGETOBJECT";
        g_mapMsgStr[WM_MENURBUTTONUP                   ] = "WM_MENURBUTTONUP";
        g_mapMsgStr[WM_MENUSELECT                      ] = "WM_MENUSELECT";
        g_mapMsgStr[WM_MOUSEACTIVATE                   ] = "WM_MOUSEACTIVATE";
        g_mapMsgStr[WM_MOUSEHOVER                      ] = "WM_MOUSEHOVER";
        g_mapMsgStr[WM_MOUSEHWHEEL                     ] = "WM_MOUSEHWHEEL";
        g_mapMsgStr[WM_MOUSELEAVE                      ] = "WM_MOUSELEAVE";
        g_mapMsgStr[WM_MOUSEMOVE                       ] = "WM_MOUSEMOVE";
        g_mapMsgStr[WM_MOUSEWHEEL                      ] = "WM_MOUSEWHEEL";
        g_mapMsgStr[WM_MOVE                            ] = "WM_MOVE";
        g_mapMsgStr[WM_MOVING                          ] = "WM_MOVING";
        g_mapMsgStr[WM_NCACTIVATE                      ] = "WM_NCACTIVATE";
        g_mapMsgStr[WM_NCCALCSIZE                      ] = "WM_NCCALCSIZE";
        g_mapMsgStr[WM_NCCREATE                        ] = "WM_NCCREATE";
        g_mapMsgStr[WM_NCDESTROY                       ] = "WM_NCDESTROY";
        g_mapMsgStr[WM_NCHITTEST                       ] = "WM_NCHITTEST";
        g_mapMsgStr[WM_NCLBUTTONDBLCLK                 ] = "WM_NCLBUTTONDBLCLK";
        g_mapMsgStr[WM_NCLBUTTONDOWN                   ] = "WM_NCLBUTTONDOWN";
        g_mapMsgStr[WM_NCLBUTTONUP                     ] = "WM_NCLBUTTONUP";
        g_mapMsgStr[WM_NCMBUTTONDBLCLK                 ] = "WM_NCMBUTTONDBLCLK";
        g_mapMsgStr[WM_NCMBUTTONDOWN                   ] = "WM_NCMBUTTONDOWN";
        g_mapMsgStr[WM_NCMBUTTONUP                     ] = "WM_NCMBUTTONUP";
        g_mapMsgStr[WM_NCMOUSEHOVER                    ] = "WM_NCMOUSEHOVER";
        g_mapMsgStr[WM_NCMOUSELEAVE                    ] = "WM_NCMOUSELEAVE";
        g_mapMsgStr[WM_NCMOUSEMOVE                     ] = "WM_NCMOUSEMOVE";
        g_mapMsgStr[WM_NCPAINT                         ] = "WM_NCPAINT";
        g_mapMsgStr[WM_NCRBUTTONDBLCLK                 ] = "WM_NCRBUTTONDBLCLK";
        g_mapMsgStr[WM_NCRBUTTONDOWN                   ] = "WM_NCRBUTTONDOWN";
        g_mapMsgStr[WM_NCRBUTTONUP                     ] = "WM_NCRBUTTONUP";
        g_mapMsgStr[WM_NCXBUTTONDBLCLK                 ] = "WM_NCXBUTTONDBLCLK";
        g_mapMsgStr[WM_NCXBUTTONDOWN                   ] = "WM_NCXBUTTONDOWN";
        g_mapMsgStr[WM_NCXBUTTONUP                     ] = "WM_NCXBUTTONUP";
        g_mapMsgStr[WM_NEXTDLGCTL                      ] = "WM_NEXTDLGCTL";
        g_mapMsgStr[WM_NEXTMENU                        ] = "WM_NEXTMENU";
        g_mapMsgStr[WM_NOTIFY                          ] = "WM_NOTIFY";
        g_mapMsgStr[WM_NOTIFYFORMAT                    ] = "WM_NOTIFYFORMAT";
        g_mapMsgStr[WM_NULL                            ] = "WM_NULL";
        g_mapMsgStr[WM_PAINT                           ] = "WM_PAINT";
        g_mapMsgStr[WM_PAINTCLIPBOARD                  ] = "WM_PAINTCLIPBOARD";
        g_mapMsgStr[WM_PAINTICON                       ] = "WM_PAINTICON";
        g_mapMsgStr[WM_PALETTECHANGED                  ] = "WM_PALETTECHANGED";
        g_mapMsgStr[WM_PALETTEISCHANGING               ] = "WM_PALETTEISCHANGING";
        g_mapMsgStr[WM_PARENTNOTIFY                    ] = "WM_PARENTNOTIFY";
        g_mapMsgStr[WM_PASTE                           ] = "WM_PASTE";
        g_mapMsgStr[WM_POPMESSAGESTRING                ] = "WM_POPMESSAGESTRING";
        g_mapMsgStr[WM_POWER                           ] = "WM_POWER";
        g_mapMsgStr[WM_POWERBROADCAST                  ] = "WM_POWERBROADCAST";
        g_mapMsgStr[WM_PRINT                           ] = "WM_PRINT";
        g_mapMsgStr[WM_PRINTCLIENT                     ] = "WM_PRINTCLIENT";
        g_mapMsgStr[WM_PSD_ENVSTAMPRECT                ] = "WM_PSD_ENVSTAMPRECT";
        g_mapMsgStr[WM_PSD_FULLPAGERECT                ] = "WM_PSD_FULLPAGERECT";
        g_mapMsgStr[WM_PSD_GREEKTEXTRECT               ] = "WM_PSD_GREEKTEXTRECT";
        g_mapMsgStr[WM_PSD_MARGINRECT                  ] = "WM_PSD_MARGINRECT";
        g_mapMsgStr[WM_PSD_MINMARGINRECT               ] = "WM_PSD_MINMARGINRECT";
        g_mapMsgStr[WM_PSD_PAGESETUPDLG                ] = "WM_PSD_PAGESETUPDLG";
        g_mapMsgStr[WM_PSD_YAFULLPAGERECT              ] = "WM_PSD_YAFULLPAGERECT";
        g_mapMsgStr[WM_QUERYAFXWNDPROC                 ] = "WM_QUERYAFXWNDPROC";
        g_mapMsgStr[WM_QUERYCENTERWND                  ] = "WM_QUERYCENTERWND";
        g_mapMsgStr[WM_QUERYDRAGICON                   ] = "WM_QUERYDRAGICON";
        g_mapMsgStr[WM_QUERYENDSESSION                 ] = "WM_QUERYENDSESSION";
        g_mapMsgStr[WM_QUERYNEWPALETTE                 ] = "WM_QUERYNEWPALETTE";
        g_mapMsgStr[WM_QUERYOPEN                       ] = "WM_QUERYOPEN";
        g_mapMsgStr[WM_QUERYUISTATE                    ] = "WM_QUERYUISTATE";
        g_mapMsgStr[WM_QUEUE_SENTINEL                  ] = "WM_QUEUE_SENTINEL";
        g_mapMsgStr[WM_QUEUESYNC                       ] = "WM_QUEUESYNC";
        g_mapMsgStr[WM_QUIT                            ] = "WM_QUIT";
        g_mapMsgStr[WM_RBUTTONDBLCLK                   ] = "WM_RBUTTONDBLCLK";
        g_mapMsgStr[WM_RBUTTONDOWN                     ] = "WM_RBUTTONDOWN";
        g_mapMsgStr[WM_RBUTTONUP                       ] = "WM_RBUTTONUP";
        g_mapMsgStr[WM_RECALCPARENT                    ] = "WM_RECALCPARENT";
        g_mapMsgStr[WM_RENDERALLFORMATS                ] = "WM_RENDERALLFORMATS";
        g_mapMsgStr[WM_RENDERFORMAT                    ] = "WM_RENDERFORMAT";
        g_mapMsgStr[WM_SETCURSOR                       ] = "WM_SETCURSOR";
        g_mapMsgStr[WM_SETFOCUS                        ] = "WM_SETFOCUS";
        g_mapMsgStr[WM_SETFONT                         ] = "WM_SETFONT";
        g_mapMsgStr[WM_SETHOTKEY                       ] = "WM_SETHOTKEY";
        g_mapMsgStr[WM_SETICON                         ] = "WM_SETICON";
        g_mapMsgStr[WM_SETMESSAGESTRING                ] = "WM_SETMESSAGESTRING";
        g_mapMsgStr[WM_SETREDRAW                       ] = "WM_SETREDRAW";
        g_mapMsgStr[WM_SETTEXT                         ] = "WM_SETTEXT";
        g_mapMsgStr[WM_SETTINGCHANGE                   ] = "WM_SETTINGCHANGE";
        g_mapMsgStr[WM_SHOWWINDOW                      ] = "WM_SHOWWINDOW";
        g_mapMsgStr[WM_SIZE                            ] = "WM_SIZE";
        g_mapMsgStr[WM_SIZECHILD                       ] = "WM_SIZECHILD";
        g_mapMsgStr[WM_SIZECLIPBOARD                   ] = "WM_SIZECLIPBOARD";
        g_mapMsgStr[WM_SIZING                          ] = "WM_SIZING";
        g_mapMsgStr[WM_SIZEPARENT                      ] = "WM_SIZEPARENT";
        g_mapMsgStr[WM_SOCKET_NOTIFY                   ] = "WM_SOCKET_NOTIFY";
        g_mapMsgStr[WM_SOCKET_DEAD                     ] = "WM_SOCKET_DEAD";
        g_mapMsgStr[WM_SPOOLERSTATUS                   ] = "WM_SPOOLERSTATUS";
        g_mapMsgStr[WM_STYLECHANGED                    ] = "WM_STYLECHANGED";
        g_mapMsgStr[WM_STYLECHANGING                   ] = "WM_STYLECHANGING";
        g_mapMsgStr[WM_SYNCPAINT                       ] = "WM_SYNCPAINT";
        g_mapMsgStr[WM_SYSCHAR                         ] = "WM_SYSCHAR";
        g_mapMsgStr[WM_SYSCOLORCHANGE                  ] = "WM_SYSCOLORCHANGE";
        g_mapMsgStr[WM_SYSCOMMAND                      ] = "WM_SYSCOMMAND";
        g_mapMsgStr[WM_SYSDEADCHAR                     ] = "WM_SYSDEADCHAR";
        g_mapMsgStr[WM_SYSKEYDOWN                      ] = "WM_SYSKEYDOWN";
        g_mapMsgStr[WM_SYSKEYUP                        ] = "WM_SYSKEYUP";
        g_mapMsgStr[WM_TCARD                           ] = "WM_TCARD";
        g_mapMsgStr[WM_THEMECHANGED                    ] = "WM_THEMECHANGED";
        g_mapMsgStr[WM_TIMECHANGE                      ] = "WM_TIMECHANGE";
        g_mapMsgStr[WM_TIMER                           ] = "WM_TIMER";
        g_mapMsgStr[WM_TOUCH                           ] = "WM_TOUCH";
        g_mapMsgStr[WM_UNDO                            ] = "WM_UNDO";
        g_mapMsgStr[WM_UNICHAR                         ] = "WM_UNICHAR";
        g_mapMsgStr[WM_UNINITMENUPOPUP                 ] = "WM_UNINITMENUPOPUP";
        g_mapMsgStr[WM_UPDATEUISTATE                   ] = "WM_UPDATEUISTATE";
        g_mapMsgStr[WM_USERCHANGED                     ] = "WM_USERCHANGED";
        g_mapMsgStr[WM_VKEYTOITEM                      ] = "WM_VKEYTOITEM";
        g_mapMsgStr[WM_VSCROLL                         ] = "WM_VSCROLL";
        g_mapMsgStr[WM_VSCROLLCLIPBOARD                ] = "WM_VSCROLLCLIPBOARD";
        g_mapMsgStr[WM_WINDOWPOSCHANGED                ] = "WM_WINDOWPOSCHANGED";
        g_mapMsgStr[WM_WINDOWPOSCHANGING               ] = "WM_WINDOWPOSCHANGING";
        g_mapMsgStr[WM_WTSSESSION_CHANGE               ] = "WM_WTSSESSION_CHANGE";
        g_mapMsgStr[WM_XBUTTONDBLCLK                   ] = "WM_XBUTTONDBLCLK";
        g_mapMsgStr[WM_XBUTTONDOWN                     ] = "WM_XBUTTONDOWN";
        g_mapMsgStr[WM_XBUTTONUP                       ] = "WM_XBUTTONUP";
    }

    if (g_mapMsgStr.find(uMsg) != g_mapMsgStr.end())
    {
        return g_mapMsgStr[uMsg];
    }

    return "";
}

DLL_EXPORT const WCHAR* GetMsgStringW(UINT uMsg)
{
    static std::map<UINT, WCHAR*> g_mapMsgStr;
    if (0 == g_mapMsgStr.size())
    {
        g_mapMsgStr[ACM_ISPLAYING                      ] = L"ACM_ISPLAYING";
        g_mapMsgStr[ACM_OPENA                          ] = L"ACM_OPENA";
        g_mapMsgStr[ACM_OPENW                          ] = L"ACM_OPENW";
        g_mapMsgStr[ACM_PLAY                           ] = L"ACM_PLAY";
        g_mapMsgStr[ACM_STOP                           ] = L"ACM_STOP";
        g_mapMsgStr[BCM_GETIDEALSIZE                   ] = L"BCM_GETIDEALSIZE";
        g_mapMsgStr[BCM_GETIMAGELIST                   ] = L"BCM_GETIMAGELIST";
        g_mapMsgStr[BCM_GETNOTE                        ] = L"BCM_GETNOTE";
        g_mapMsgStr[BCM_GETNOTELENGTH                  ] = L"BCM_GETNOTELENGTH";
        g_mapMsgStr[BCM_GETSPLITINFO                   ] = L"BCM_GETSPLITINFO";
        g_mapMsgStr[BCM_GETTEXTMARGIN                  ] = L"BCM_GETTEXTMARGIN";
        g_mapMsgStr[BCM_SETDROPDOWNSTATE               ] = L"BCM_SETDROPDOWNSTATE";
        g_mapMsgStr[BCM_SETIMAGELIST                   ] = L"BCM_SETIMAGELIST";
        g_mapMsgStr[BCM_SETNOTE                        ] = L"BCM_SETNOTE";
        g_mapMsgStr[BCM_SETSHIELD                      ] = L"BCM_SETSHIELD";
        g_mapMsgStr[BCM_SETSPLITINFO                   ] = L"BCM_SETSPLITINFO";
        g_mapMsgStr[BCM_SETTEXTMARGIN                  ] = L"BCM_SETTEXTMARGIN";
        g_mapMsgStr[BM_CLICK                           ] = L"BM_CLICK";
        g_mapMsgStr[BM_GETCHECK                        ] = L"BM_GETCHECK";
        g_mapMsgStr[BM_GETIMAGE                        ] = L"BM_GETIMAGE";
        g_mapMsgStr[BM_GETSTATE                        ] = L"BM_GETSTATE";
        g_mapMsgStr[BM_SETCHECK                        ] = L"BM_SETCHECK";
        g_mapMsgStr[BM_SETDONTCLICK                    ] = L"BM_SETDONTCLICK";
        g_mapMsgStr[BM_SETIMAGE                        ] = L"BM_SETIMAGE";
        g_mapMsgStr[BM_SETSTATE                        ] = L"BM_SETSTATE";
        g_mapMsgStr[BM_SETSTYLE                        ] = L"BM_SETSTYLE";
        g_mapMsgStr[CB_ADDSTRING                       ] = L"CB_ADDSTRING";
        g_mapMsgStr[CB_DELETESTRING                    ] = L"CB_DELETESTRING";
        g_mapMsgStr[CB_DIR                             ] = L"CB_DIR";
        g_mapMsgStr[CB_FINDSTRING                      ] = L"CB_FINDSTRING";
        g_mapMsgStr[CB_FINDSTRINGEXACT                 ] = L"CB_FINDSTRINGEXACT";
        g_mapMsgStr[CB_GETCOMBOBOXINFO                 ] = L"CB_GETCOMBOBOXINFO";
        g_mapMsgStr[CB_GETCOUNT                        ] = L"CB_GETCOUNT";
        g_mapMsgStr[CB_GETCUEBANNER                    ] = L"CB_GETCUEBANNER";
        g_mapMsgStr[CB_GETCURSEL                       ] = L"CB_GETCURSEL";
        g_mapMsgStr[CB_GETDROPPEDCONTROLRECT           ] = L"CB_GETDROPPEDCONTROLRECT";
        g_mapMsgStr[CB_GETDROPPEDSTATE                 ] = L"CB_GETDROPPEDSTATE";
        g_mapMsgStr[CB_GETDROPPEDWIDTH                 ] = L"CB_GETDROPPEDWIDTH";
        g_mapMsgStr[CB_GETEDITSEL                      ] = L"CB_GETEDITSEL";
        g_mapMsgStr[CB_GETEXTENDEDUI                   ] = L"CB_GETEXTENDEDUI";
        g_mapMsgStr[CB_GETHORIZONTALEXTENT             ] = L"CB_GETHORIZONTALEXTENT";
        g_mapMsgStr[CB_GETITEMDATA                     ] = L"CB_GETITEMDATA";
        g_mapMsgStr[CB_GETITEMHEIGHT                   ] = L"CB_GETITEMHEIGHT";
        g_mapMsgStr[CB_GETLBTEXT                       ] = L"CB_GETLBTEXT";
        g_mapMsgStr[CB_GETLBTEXTLEN                    ] = L"CB_GETLBTEXTLEN";
        g_mapMsgStr[CB_GETLOCALE                       ] = L"CB_GETLOCALE";
        g_mapMsgStr[CB_GETMINVISIBLE                   ] = L"CB_GETMINVISIBLE";
        g_mapMsgStr[CB_GETTOPINDEX                     ] = L"CB_GETTOPINDEX";
        g_mapMsgStr[CB_INITSTORAGE                     ] = L"CB_INITSTORAGE";
        g_mapMsgStr[CB_INSERTSTRING                    ] = L"CB_INSERTSTRING";
        g_mapMsgStr[CB_LIMITTEXT                       ] = L"CB_LIMITTEXT";
        g_mapMsgStr[CB_MULTIPLEADDSTRING               ] = L"CB_MULTIPLEADDSTRING";
        g_mapMsgStr[CB_RESETCONTENT                    ] = L"CB_RESETCONTENT";
        g_mapMsgStr[CB_SELECTSTRING                    ] = L"CB_SELECTSTRING";
        g_mapMsgStr[CB_SETCUEBANNER                    ] = L"CB_SETCUEBANNER";
        g_mapMsgStr[CB_SETCURSEL                       ] = L"CB_SETCURSEL";
        g_mapMsgStr[CB_SETDROPPEDWIDTH                 ] = L"CB_SETDROPPEDWIDTH";
        g_mapMsgStr[CB_SETEDITSEL                      ] = L"CB_SETEDITSEL";
        g_mapMsgStr[CB_SETEXTENDEDUI                   ] = L"CB_SETEXTENDEDUI";
        g_mapMsgStr[CB_SETHORIZONTALEXTENT             ] = L"CB_SETHORIZONTALEXTENT";
        g_mapMsgStr[CB_SETITEMDATA                     ] = L"CB_SETITEMDATA";
        g_mapMsgStr[CB_SETITEMHEIGHT                   ] = L"CB_SETITEMHEIGHT";
        g_mapMsgStr[CB_SETLOCALE                       ] = L"CB_SETLOCALE";
        g_mapMsgStr[CB_SETMINVISIBLE                   ] = L"CB_SETMINVISIBLE";
        g_mapMsgStr[CB_SETTOPINDEX                     ] = L"CB_SETTOPINDEX";
        g_mapMsgStr[CB_SHOWDROPDOWN                    ] = L"CB_SHOWDROPDOWN";
        g_mapMsgStr[CBEM_GETCOMBOCONTROL               ] = L"CBEM_GETCOMBOCONTROL";
        g_mapMsgStr[CBEM_GETEDITCONTROL                ] = L"CBEM_GETEDITCONTROL";
        g_mapMsgStr[CBEM_GETEXTENDEDSTYLE              ] = L"CBEM_GETEXTENDEDSTYLE";
        g_mapMsgStr[CBEM_GETIMAGELIST                  ] = L"CBEM_GETIMAGELIST";
        g_mapMsgStr[CBEM_GETITEMA                      ] = L"CBEM_GETITEMA";
        g_mapMsgStr[CBEM_GETITEMW                      ] = L"CBEM_GETITEMW";
        g_mapMsgStr[CBEM_HASEDITCHANGED                ] = L"CBEM_HASEDITCHANGED";
        g_mapMsgStr[CBEM_INSERTITEMA                   ] = L"CBEM_INSERTITEMA";
        g_mapMsgStr[CBEM_INSERTITEMW                   ] = L"CBEM_INSERTITEMW";
        g_mapMsgStr[CBEM_SETEXSTYLE                    ] = L"CBEM_SETEXSTYLE";
        g_mapMsgStr[CBEM_SETEXTENDEDSTYLE              ] = L"CBEM_SETEXTENDEDSTYLE";
        g_mapMsgStr[CBEM_SETIMAGELIST                  ] = L"CBEM_SETIMAGELIST";
        g_mapMsgStr[CBEM_SETITEMA                      ] = L"CBEM_SETITEMA";
        g_mapMsgStr[CBEM_SETITEMW                      ] = L"CBEM_SETITEMW";
        g_mapMsgStr[CDM_GETFILEPATH                    ] = L"CDM_GETFILEPATH";
        g_mapMsgStr[CDM_GETFOLDERIDLIST                ] = L"CDM_GETFOLDERIDLIST";
        g_mapMsgStr[CDM_GETFOLDERPATH                  ] = L"CDM_GETFOLDERPATH";
        g_mapMsgStr[CDM_GETSPEC                        ] = L"CDM_GETSPEC";
        g_mapMsgStr[CDM_HIDECONTROL                    ] = L"CDM_HIDECONTROL";
        g_mapMsgStr[CDM_SETCONTROLTEXT                 ] = L"CDM_SETCONTROLTEXT";
        g_mapMsgStr[CDM_SETDEFEXT                      ] = L"CDM_SETDEFEXT";
        g_mapMsgStr[CCM_DPISCALE                       ] = L"CCM_DPISCALE";
        g_mapMsgStr[CCM_GETCOLORSCHEME                 ] = L"CCM_GETCOLORSCHEME";
        g_mapMsgStr[CCM_GETDROPTARGET                  ] = L"CCM_GETDROPTARGET";
        g_mapMsgStr[CCM_GETUNICODEFORMAT               ] = L"CCM_GETUNICODEFORMAT";
        g_mapMsgStr[CCM_GETVERSION                     ] = L"CCM_GETVERSION";
        g_mapMsgStr[CCM_SETBKCOLOR                     ] = L"CCM_SETBKCOLOR";
        g_mapMsgStr[CCM_SETCOLORSCHEME                 ] = L"CCM_SETCOLORSCHEME";
        g_mapMsgStr[CCM_SETNOTIFYWINDOW                ] = L"CCM_SETNOTIFYWINDOW";
        g_mapMsgStr[CCM_SETUNICODEFORMAT               ] = L"CCM_SETUNICODEFORMAT";
        g_mapMsgStr[CCM_SETVERSION                     ] = L"CCM_SETVERSION";
        g_mapMsgStr[CCM_SETWINDOWTHEME                 ] = L"CCM_SETWINDOWTHEME";
        g_mapMsgStr[DL_BEGINDRAG                       ] = L"DL_BEGINDRAG";
        g_mapMsgStr[DL_CANCELDRAG                      ] = L"DL_CANCELDRAG";
        g_mapMsgStr[DL_DRAGGING                        ] = L"DL_DRAGGING";
        g_mapMsgStr[DL_DROPPED                         ] = L"DL_DROPPED";
        g_mapMsgStr[DM_GETDEFID                        ] = L"DM_GETDEFID";
        g_mapMsgStr[DM_REPOSITION                      ] = L"DM_REPOSITION";
        g_mapMsgStr[DM_SETDEFID                        ] = L"DM_SETDEFID";
        g_mapMsgStr[DTM_CLOSEMONTHCAL                  ] = L"DTM_CLOSEMONTHCAL";
        g_mapMsgStr[DTM_GETDATETIMEPICKERINFO          ] = L"DTM_GETDATETIMEPICKERINFO";
        g_mapMsgStr[DTM_GETIDEALSIZE                   ] = L"DTM_GETIDEALSIZE";
        g_mapMsgStr[DTM_GETMCCOLOR                     ] = L"DTM_GETMCCOLOR";
        g_mapMsgStr[DTM_GETMCFONT                      ] = L"DTM_GETMCFONT";
        g_mapMsgStr[DTM_GETMCSTYLE                     ] = L"DTM_GETMCSTYLE";
        g_mapMsgStr[DTM_GETMONTHCAL                    ] = L"DTM_GETMONTHCAL";
        g_mapMsgStr[DTM_GETRANGE                       ] = L"DTM_GETRANGE";
        g_mapMsgStr[DTM_GETSYSTEMTIME                  ] = L"DTM_GETSYSTEMTIME";
        g_mapMsgStr[DTM_SETFORMATA                     ] = L"DTM_SETFORMATA";
        g_mapMsgStr[DTM_SETFORMATW                     ] = L"DTM_SETFORMATW";
        g_mapMsgStr[DTM_SETMCCOLOR                     ] = L"DTM_SETMCCOLOR";
        g_mapMsgStr[DTM_SETMCFONT                      ] = L"DTM_SETMCFONT";
        g_mapMsgStr[DTM_SETMCSTYLE                     ] = L"DTM_SETMCSTYLE";
        g_mapMsgStr[DTM_SETRANGE                       ] = L"DTM_SETRANGE";
        g_mapMsgStr[DTM_SETSYSTEMTIME                  ] = L"DTM_SETSYSTEMTIME";
        g_mapMsgStr[EM_AUTOURLDETECT                   ] = L"EM_AUTOURLDETECT";
        g_mapMsgStr[EM_CANPASTE                        ] = L"EM_CANPASTE";
        g_mapMsgStr[EM_CANREDO                         ] = L"EM_CANREDO";
        g_mapMsgStr[EM_CANUNDO                         ] = L"EM_CANUNDO";
        g_mapMsgStr[EM_CHARFROMPOS                     ] = L"EM_CHARFROMPOS";
        g_mapMsgStr[EM_CONVPOSITION                    ] = L"EM_CONVPOSITION";
        g_mapMsgStr[EM_DISPLAYBAND                     ] = L"EM_DISPLAYBAND";
        g_mapMsgStr[EM_EMPTYUNDOBUFFER                 ] = L"EM_EMPTYUNDOBUFFER";
        g_mapMsgStr[EM_EXGETSEL                        ] = L"EM_EXGETSEL";
        g_mapMsgStr[EM_EXLIMITTEXT                     ] = L"EM_EXLIMITTEXT";
        g_mapMsgStr[EM_EXLINEFROMCHAR                  ] = L"EM_EXLINEFROMCHAR";
        g_mapMsgStr[EM_EXSETSEL                        ] = L"EM_EXSETSEL";
        g_mapMsgStr[EM_FINDTEXT                        ] = L"EM_FINDTEXT";
        g_mapMsgStr[EM_FINDTEXTW                       ] = L"EM_FINDTEXTW";
        g_mapMsgStr[EM_FINDTEXTEX                      ] = L"EM_FINDTEXTEX";
        g_mapMsgStr[EM_FINDTEXTEXW                     ] = L"EM_FINDTEXTEXW";
        g_mapMsgStr[EM_FINDWORDBREAK                   ] = L"EM_FINDWORDBREAK";
        g_mapMsgStr[EM_FMTLINES                        ] = L"EM_FMTLINES";
        g_mapMsgStr[EM_FORMATRANGE                     ] = L"EM_FORMATRANGE";
        g_mapMsgStr[EM_GETAUTOURLDETECT                ] = L"EM_GETAUTOURLDETECT";
        g_mapMsgStr[EM_GETBIDIOPTIONS                  ] = L"EM_GETBIDIOPTIONS";
        g_mapMsgStr[EM_GETCHARFORMAT                   ] = L"EM_GETCHARFORMAT";
        g_mapMsgStr[EM_GETCTFMODEBIAS                  ] = L"EM_GETCTFMODEBIAS";
        g_mapMsgStr[EM_GETCTFOPENSTATUS                ] = L"EM_GETCTFOPENSTATUS";
        g_mapMsgStr[EM_GETCUEBANNER                    ] = L"EM_GETCUEBANNER";
        g_mapMsgStr[EM_GETEDITSTYLE                    ] = L"EM_GETEDITSTYLE";
        g_mapMsgStr[EM_GETEVENTMASK                    ] = L"EM_GETEVENTMASK";
        g_mapMsgStr[EM_GETFIRSTVISIBLELINE             ] = L"EM_GETFIRSTVISIBLELINE";
        g_mapMsgStr[EM_GETHANDLE                       ] = L"EM_GETHANDLE";
        g_mapMsgStr[EM_GETHILITE                       ] = L"EM_GETHILITE";
        g_mapMsgStr[EM_GETHYPHENATEINFO                ] = L"EM_GETHYPHENATEINFO";
        g_mapMsgStr[EM_GETIMECOLOR                     ] = L"EM_GETIMECOLOR";
        g_mapMsgStr[EM_GETIMECOMPMODE                  ] = L"EM_GETIMECOMPMODE";
        g_mapMsgStr[EM_GETIMECOMPTEXT                  ] = L"EM_GETIMECOMPTEXT";
        g_mapMsgStr[EM_GETIMEMODEBIAS                  ] = L"EM_GETIMEMODEBIAS";
        g_mapMsgStr[EM_GETIMEOPTIONS                   ] = L"EM_GETIMEOPTIONS";
        g_mapMsgStr[EM_GETIMEPROPERTY                  ] = L"EM_GETIMEPROPERTY";
        g_mapMsgStr[EM_GETIMESTATUS                    ] = L"EM_GETIMESTATUS";
        g_mapMsgStr[EM_GETLANGOPTIONS                  ] = L"EM_GETLANGOPTIONS";
        g_mapMsgStr[EM_GETLIMITTEXT                    ] = L"EM_GETLIMITTEXT";
        g_mapMsgStr[EM_GETLINE                         ] = L"EM_GETLINE";
        g_mapMsgStr[EM_GETLINECOUNT                    ] = L"EM_GETLINECOUNT";
        g_mapMsgStr[EM_GETMARGINS                      ] = L"EM_GETMARGINS";
        g_mapMsgStr[EM_GETMODIFY                       ] = L"EM_GETMODIFY";
        g_mapMsgStr[EM_GETOLEINTERFACE                 ] = L"EM_GETOLEINTERFACE";
        g_mapMsgStr[EM_GETOPTIONS                      ] = L"EM_GETOPTIONS";
        g_mapMsgStr[EM_GETPAGE                         ] = L"EM_GETPAGE";
        g_mapMsgStr[EM_GETPAGEROTATE                   ] = L"EM_GETPAGEROTATE";
        g_mapMsgStr[EM_GETPARAFORMAT                   ] = L"EM_GETPARAFORMAT";
        g_mapMsgStr[EM_GETPASSWORDCHAR                 ] = L"EM_GETPASSWORDCHAR";
        g_mapMsgStr[EM_GETPUNCTUATION                  ] = L"EM_GETPUNCTUATION";
        g_mapMsgStr[EM_GETQUERYRTFOBJ                  ] = L"EM_GETQUERYRTFOBJ";
        g_mapMsgStr[EM_GETRECT                         ] = L"EM_GETRECT";
        g_mapMsgStr[EM_GETREDONAME                     ] = L"EM_GETREDONAME";
        g_mapMsgStr[EM_GETSCROLLPOS                    ] = L"EM_GETSCROLLPOS";
        g_mapMsgStr[EM_GETSEL                          ] = L"EM_GETSEL";
        g_mapMsgStr[EM_GETSELTEXT                      ] = L"EM_GETSELTEXT";
        g_mapMsgStr[EM_GETTEXTEX                       ] = L"EM_GETTEXTEX";
        g_mapMsgStr[EM_GETTEXTLENGTHEX                 ] = L"EM_GETTEXTLENGTHEX";
        g_mapMsgStr[EM_GETTEXTMODE                     ] = L"EM_GETTEXTMODE";
        g_mapMsgStr[EM_GETTEXTRANGE                    ] = L"EM_GETTEXTRANGE";
        g_mapMsgStr[EM_GETTHUMB                        ] = L"EM_GETTHUMB";
        g_mapMsgStr[EM_GETTYPOGRAPHYOPTIONS            ] = L"EM_GETTYPOGRAPHYOPTIONS";
        g_mapMsgStr[EM_GETUNDONAME                     ] = L"EM_GETUNDONAME";
        g_mapMsgStr[EM_GETVIEWKIND                     ] = L"EM_GETVIEWKIND";
        g_mapMsgStr[EM_GETWORDBREAKPROC                ] = L"EM_GETWORDBREAKPROC";
        g_mapMsgStr[EM_GETWORDBREAKPROCEX              ] = L"EM_GETWORDBREAKPROCEX";
        g_mapMsgStr[EM_GETWORDWRAPMODE                 ] = L"EM_GETWORDWRAPMODE";
        g_mapMsgStr[EM_GETZOOM                         ] = L"EM_GETZOOM";
        g_mapMsgStr[EM_HIDEBALLOONTIP                  ] = L"EM_HIDEBALLOONTIP";
        g_mapMsgStr[EM_HIDESELECTION                   ] = L"EM_HIDESELECTION";
        g_mapMsgStr[EM_ISIME                           ] = L"EM_ISIME";
        g_mapMsgStr[EM_LINEFROMCHAR                    ] = L"EM_LINEFROMCHAR";
        g_mapMsgStr[EM_LINEINDEX                       ] = L"EM_LINEINDEX";
        g_mapMsgStr[EM_LINELENGTH                      ] = L"EM_LINELENGTH";
        g_mapMsgStr[EM_LINESCROLL                      ] = L"EM_LINESCROLL";
        g_mapMsgStr[EM_NOSETFOCUS                      ] = L"EM_NOSETFOCUS";
        g_mapMsgStr[EM_OUTLINE                         ] = L"EM_OUTLINE";
        g_mapMsgStr[EM_PASTESPECIAL                    ] = L"EM_PASTESPECIAL";
        g_mapMsgStr[EM_POSFROMCHAR                     ] = L"EM_POSFROMCHAR";
        g_mapMsgStr[EM_RECONVERSION                    ] = L"EM_RECONVERSION";
        g_mapMsgStr[EM_REDO                            ] = L"EM_REDO";
        g_mapMsgStr[EM_REPLACESEL                      ] = L"EM_REPLACESEL";
        g_mapMsgStr[EM_REQUESTRESIZE                   ] = L"EM_REQUESTRESIZE";
        g_mapMsgStr[EM_SCROLL                          ] = L"EM_SCROLL";
        g_mapMsgStr[EM_SCROLLCARET                     ] = L"EM_SCROLLCARET";
        g_mapMsgStr[EM_SELECTIONTYPE                   ] = L"EM_SELECTIONTYPE";
        g_mapMsgStr[EM_SETBIDIOPTIONS                  ] = L"EM_SETBIDIOPTIONS";
        g_mapMsgStr[EM_SETBKGNDCOLOR                   ] = L"EM_SETBKGNDCOLOR";
        g_mapMsgStr[EM_SETCHARFORMAT                   ] = L"EM_SETCHARFORMAT";
        g_mapMsgStr[EM_SETCTFMODEBIAS                  ] = L"EM_SETCTFMODEBIAS";
        g_mapMsgStr[EM_SETCTFOPENSTATUS                ] = L"EM_SETCTFOPENSTATUS";
        g_mapMsgStr[EM_SETCUEBANNER                    ] = L"EM_SETCUEBANNER";
        g_mapMsgStr[EM_SETEDITSTYLE                    ] = L"EM_SETEDITSTYLE";
        g_mapMsgStr[EM_SETEVENTMASK                    ] = L"EM_SETEVENTMASK";
        g_mapMsgStr[EM_SETFONTSIZE                     ] = L"EM_SETFONTSIZE";
        g_mapMsgStr[EM_SETHANDLE                       ] = L"EM_SETHANDLE";
        g_mapMsgStr[EM_SETHILITE                       ] = L"EM_SETHILITE";
        g_mapMsgStr[EM_SETHYPHENATEINFO                ] = L"EM_SETHYPHENATEINFO";
        g_mapMsgStr[EM_SETIMECOLOR                     ] = L"EM_SETIMECOLOR";
        g_mapMsgStr[EM_SETIMEMODEBIAS                  ] = L"EM_SETIMEMODEBIAS";
        g_mapMsgStr[EM_SETIMEOPTIONS                   ] = L"EM_SETIMEOPTIONS";
        g_mapMsgStr[EM_SETIMESTATUS                    ] = L"EM_SETIMESTATUS";
        g_mapMsgStr[EM_SETLANGOPTIONS                  ] = L"EM_SETLANGOPTIONS";
        g_mapMsgStr[EM_SETLIMITTEXT                    ] = L"EM_SETLIMITTEXT";
        g_mapMsgStr[EM_SETMARGINS                      ] = L"EM_SETMARGINS";
        g_mapMsgStr[EM_SETMODIFY                       ] = L"EM_SETMODIFY";
        g_mapMsgStr[EM_SETOLECALLBACK                  ] = L"EM_SETOLECALLBACK";
        g_mapMsgStr[EM_SETOPTIONS                      ] = L"EM_SETOPTIONS";
        g_mapMsgStr[EM_SETPAGE                         ] = L"EM_SETPAGE";
        g_mapMsgStr[EM_SETPAGEROTATE                   ] = L"EM_SETPAGEROTATE";
        g_mapMsgStr[EM_SETPALETTE                      ] = L"EM_SETPALETTE";
        g_mapMsgStr[EM_SETPARAFORMAT                   ] = L"EM_SETPARAFORMAT";
        g_mapMsgStr[EM_SETPASSWORDCHAR                 ] = L"EM_SETPASSWORDCHAR";
        g_mapMsgStr[EM_SETPUNCTUATION                  ] = L"EM_SETPUNCTUATION";
        g_mapMsgStr[EM_SETQUERYRTFOBJ                  ] = L"EM_SETQUERYRTFOBJ";
        g_mapMsgStr[EM_SETREADONLY                     ] = L"EM_SETREADONLY";
        g_mapMsgStr[EM_SETRECT                         ] = L"EM_SETRECT";
        g_mapMsgStr[EM_SETRECTNP                       ] = L"EM_SETRECTNP";
        g_mapMsgStr[EM_SETSCROLLPOS                    ] = L"EM_SETSCROLLPOS";
        g_mapMsgStr[EM_SETSEL                          ] = L"EM_SETSEL";
        g_mapMsgStr[EM_SETTABSTOPS                     ] = L"EM_SETTABSTOPS";
        g_mapMsgStr[EM_SETTARGETDEVICE                 ] = L"EM_SETTARGETDEVICE";
        g_mapMsgStr[EM_SETTEXTEX                       ] = L"EM_SETTEXTEX";
        g_mapMsgStr[EM_SETTEXTMODE                     ] = L"EM_SETTEXTMODE";
        g_mapMsgStr[EM_SETTYPOGRAPHYOPTIONS            ] = L"EM_SETTYPOGRAPHYOPTIONS";
        g_mapMsgStr[EM_SETUNDOLIMIT                    ] = L"EM_SETUNDOLIMIT";
        g_mapMsgStr[EM_SETVIEWKIND                     ] = L"EM_SETVIEWKIND";
        g_mapMsgStr[EM_SETWORDBREAKPROC                ] = L"EM_SETWORDBREAKPROC";
        g_mapMsgStr[EM_SETWORDBREAKPROCEX              ] = L"EM_SETWORDBREAKPROCEX";
        g_mapMsgStr[EM_SETWORDWRAPMODE                 ] = L"EM_SETWORDWRAPMODE";
        g_mapMsgStr[EM_SETZOOM                         ] = L"EM_SETZOOM";
        g_mapMsgStr[EM_SHOWBALLOONTIP                  ] = L"EM_SHOWBALLOONTIP";
        g_mapMsgStr[EM_SHOWSCROLLBAR                   ] = L"EM_SHOWSCROLLBAR";
        g_mapMsgStr[EM_STOPGROUPTYPING                 ] = L"EM_STOPGROUPTYPING";
        g_mapMsgStr[EM_STREAMIN                        ] = L"EM_STREAMIN";
        g_mapMsgStr[EM_STREAMOUT                       ] = L"EM_STREAMOUT";
        g_mapMsgStr[EM_TAKEFOCUS                       ] = L"EM_TAKEFOCUS";
        g_mapMsgStr[EM_UNDO                            ] = L"EM_UNDO";
        g_mapMsgStr[HDM_CREATEDRAGIMAGE                ] = L"HDM_CREATEDRAGIMAGE";
        g_mapMsgStr[HDM_CLEARFILTER                    ] = L"HDM_CLEARFILTER";
        g_mapMsgStr[HDM_DELETEITEM                     ] = L"HDM_DELETEITEM";
        g_mapMsgStr[HDM_EDITFILTER                     ] = L"HDM_EDITFILTER";
        g_mapMsgStr[HDM_GETBITMAPMARGIN                ] = L"HDM_GETBITMAPMARGIN";
        g_mapMsgStr[HDM_GETFOCUSEDITEM                 ] = L"HDM_GETFOCUSEDITEM";
        g_mapMsgStr[HDM_GETIMAGELIST                   ] = L"HDM_GETIMAGELIST";
        g_mapMsgStr[HDM_GETITEMA                       ] = L"HDM_GETITEMA";
        g_mapMsgStr[HDM_GETITEMW                       ] = L"HDM_GETITEMW";
        g_mapMsgStr[HDM_GETITEMCOUNT                   ] = L"HDM_GETITEMCOUNT";
        g_mapMsgStr[HDM_GETITEMDROPDOWNRECT            ] = L"HDM_GETITEMDROPDOWNRECT";
        g_mapMsgStr[HDM_GETITEMRECT                    ] = L"HDM_GETITEMRECT";
        g_mapMsgStr[HDM_GETORDERARRAY                  ] = L"HDM_GETORDERARRAY";
        g_mapMsgStr[HDM_GETOVERFLOWRECT                ] = L"HDM_GETOVERFLOWRECT";
        g_mapMsgStr[HDM_HITTEST                        ] = L"HDM_HITTEST";
        g_mapMsgStr[HDM_INSERTITEMA                    ] = L"HDM_INSERTITEMA";
        g_mapMsgStr[HDM_INSERTITEMW                    ] = L"HDM_INSERTITEMW";
        g_mapMsgStr[HDM_LAYOUT                         ] = L"HDM_LAYOUT";
        g_mapMsgStr[HDM_ORDERTOINDEX                   ] = L"HDM_ORDERTOINDEX";
        g_mapMsgStr[HDM_SETBITMAPMARGIN                ] = L"HDM_SETBITMAPMARGIN";
        g_mapMsgStr[HDM_SETFILTERCHANGETIMEOUT         ] = L"HDM_SETFILTERCHANGETIMEOUT";
        g_mapMsgStr[HDM_SETFOCUSEDITEM                 ] = L"HDM_SETFOCUSEDITEM";
        g_mapMsgStr[HDM_SETHOTDIVIDER                  ] = L"HDM_SETHOTDIVIDER";
        g_mapMsgStr[HDM_SETIMAGELIST                   ] = L"HDM_SETIMAGELIST";
        g_mapMsgStr[HDM_SETITEMA                       ] = L"HDM_SETITEMA";
        g_mapMsgStr[HDM_SETITEMW                       ] = L"HDM_SETITEMW";
        g_mapMsgStr[HDM_SETORDERARRAY                  ] = L"HDM_SETORDERARRAY";
        g_mapMsgStr[HKM_GETHOTKEY                      ] = L"HKM_GETHOTKEY";
        g_mapMsgStr[HKM_SETHOTKEY                      ] = L"HKM_SETHOTKEY";
        g_mapMsgStr[HKM_SETRULES                       ] = L"HKM_SETRULES";
        g_mapMsgStr[IPM_CLEARADDRESS                   ] = L"IPM_CLEARADDRESS";
        g_mapMsgStr[IPM_GETADDRESS                     ] = L"IPM_GETADDRESS";
        g_mapMsgStr[IPM_ISBLANK                        ] = L"IPM_ISBLANK";
        g_mapMsgStr[IPM_SETADDRESS                     ] = L"IPM_SETADDRESS";
        g_mapMsgStr[IPM_SETFOCUS                       ] = L"IPM_SETFOCUS";
        g_mapMsgStr[IPM_SETRANGE                       ] = L"IPM_SETRANGE";
        g_mapMsgStr[LB_ADDFILE                         ] = L"LB_ADDFILE";
        g_mapMsgStr[LB_ADDSTRING                       ] = L"LB_ADDSTRING";
        g_mapMsgStr[LB_DELETESTRING                    ] = L"LB_DELETESTRING";
        g_mapMsgStr[LB_DIR                             ] = L"LB_DIR";
        g_mapMsgStr[LB_FINDSTRING                      ] = L"LB_FINDSTRING";
        g_mapMsgStr[LB_FINDSTRINGEXACT                 ] = L"LB_FINDSTRINGEXACT";
        g_mapMsgStr[LB_GETANCHORINDEX                  ] = L"LB_GETANCHORINDEX";
        g_mapMsgStr[LB_GETCARETINDEX                   ] = L"LB_GETCARETINDEX";
        g_mapMsgStr[LB_GETCOUNT                        ] = L"LB_GETCOUNT";
        g_mapMsgStr[LB_GETCURSEL                       ] = L"LB_GETCURSEL";
        g_mapMsgStr[LB_GETHORIZONTALEXTENT             ] = L"LB_GETHORIZONTALEXTENT";
        g_mapMsgStr[LB_GETITEMDATA                     ] = L"LB_GETITEMDATA";
        g_mapMsgStr[LB_GETITEMHEIGHT                   ] = L"LB_GETITEMHEIGHT";
        g_mapMsgStr[LB_GETITEMRECT                     ] = L"LB_GETITEMRECT";
        g_mapMsgStr[LB_GETLISTBOXINFO                  ] = L"LB_GETLISTBOXINFO";
        g_mapMsgStr[LB_GETLOCALE                       ] = L"LB_GETLOCALE";
        g_mapMsgStr[LB_GETSEL                          ] = L"LB_GETSEL";
        g_mapMsgStr[LB_GETSELCOUNT                     ] = L"LB_GETSELCOUNT";
        g_mapMsgStr[LB_GETSELITEMS                     ] = L"LB_GETSELITEMS";
        g_mapMsgStr[LB_GETTEXT                         ] = L"LB_GETTEXT";
        g_mapMsgStr[LB_GETTEXTLEN                      ] = L"LB_GETTEXTLEN";
        g_mapMsgStr[LB_GETTOPINDEX                     ] = L"LB_GETTOPINDEX";
        g_mapMsgStr[LB_INITSTORAGE                     ] = L"LB_INITSTORAGE";
        g_mapMsgStr[LB_INSERTSTRING                    ] = L"LB_INSERTSTRING";
        g_mapMsgStr[LB_ITEMFROMPOINT                   ] = L"LB_ITEMFROMPOINT";
        g_mapMsgStr[LB_MULTIPLEADDSTRING               ] = L"LB_MULTIPLEADDSTRING";
        g_mapMsgStr[LB_RESETCONTENT                    ] = L"LB_RESETCONTENT";
        g_mapMsgStr[LB_SELECTSTRING                    ] = L"LB_SELECTSTRING";
        g_mapMsgStr[LB_SELITEMRANGE                    ] = L"LB_SELITEMRANGE";
        g_mapMsgStr[LB_SELITEMRANGEEX                  ] = L"LB_SELITEMRANGEEX";
        g_mapMsgStr[LB_SETANCHORINDEX                  ] = L"LB_SETANCHORINDEX";
        g_mapMsgStr[LB_SETCARETINDEX                   ] = L"LB_SETCARETINDEX";
        g_mapMsgStr[LB_SETCOLUMNWIDTH                  ] = L"LB_SETCOLUMNWIDTH";
        g_mapMsgStr[LB_SETCOUNT                        ] = L"LB_SETCOUNT";
        g_mapMsgStr[LB_SETCURSEL                       ] = L"LB_SETCURSEL";
        g_mapMsgStr[LB_SETHORIZONTALEXTENT             ] = L"LB_SETHORIZONTALEXTENT";
        g_mapMsgStr[LB_SETITEMDATA                     ] = L"LB_SETITEMDATA";
        g_mapMsgStr[LB_SETITEMHEIGHT                   ] = L"LB_SETITEMHEIGHT";
        g_mapMsgStr[LB_SETLOCALE                       ] = L"LB_SETLOCALE";
        g_mapMsgStr[LB_SETSEL                          ] = L"LB_SETSEL";
        g_mapMsgStr[LB_SETTABSTOPS                     ] = L"LB_SETTABSTOPS";
        g_mapMsgStr[LB_SETTOPINDEX                     ] = L"LB_SETTOPINDEX";
        g_mapMsgStr[LM_GETIDEALHEIGHT                  ] = L"LM_GETIDEALHEIGHT";
        g_mapMsgStr[LM_GETITEM                         ] = L"LM_GETITEM";
        g_mapMsgStr[LM_HITTEST                         ] = L"LM_HITTEST";
        g_mapMsgStr[LM_SETITEM                         ] = L"LM_SETITEM";
        g_mapMsgStr[LVM_APPROXIMATEVIEWRECT            ] = L"LVM_APPROXIMATEVIEWRECT";
        g_mapMsgStr[LVM_ARRANGE                        ] = L"LVM_ARRANGE";
        g_mapMsgStr[LVM_CANCELEDITLABEL                ] = L"LVM_CANCELEDITLABEL";
        g_mapMsgStr[LVM_CREATEDRAGIMAGE                ] = L"LVM_CREATEDRAGIMAGE";
        g_mapMsgStr[LVM_DELETEALLITEMS                 ] = L"LVM_DELETEALLITEMS";
        g_mapMsgStr[LVM_DELETECOLUMN                   ] = L"LVM_DELETECOLUMN";
        g_mapMsgStr[LVM_DELETEITEM                     ] = L"LVM_DELETEITEM";
        g_mapMsgStr[LVM_EDITLABELA                     ] = L"LVM_EDITLABELA";
        g_mapMsgStr[LVM_EDITLABELW                     ] = L"LVM_EDITLABELW";
        g_mapMsgStr[LVM_ENABLEGROUPVIEW                ] = L"LVM_ENABLEGROUPVIEW";
        g_mapMsgStr[LVM_ENSUREVISIBLE                  ] = L"LVM_ENSUREVISIBLE";
        g_mapMsgStr[LVM_FINDITEMA                      ] = L"LVM_FINDITEMA";
        g_mapMsgStr[LVM_FINDITEMW                      ] = L"LVM_FINDITEMW";
        g_mapMsgStr[LVM_GETBKCOLOR                     ] = L"LVM_GETBKCOLOR";
        g_mapMsgStr[LVM_GETBKIMAGEA                    ] = L"LVM_GETBKIMAGEA";
        g_mapMsgStr[LVM_GETBKIMAGEW                    ] = L"LVM_GETBKIMAGEW";
        g_mapMsgStr[LVM_GETCALLBACKMASK                ] = L"LVM_GETCALLBACKMASK";
        g_mapMsgStr[LVM_GETCOLUMNA                     ] = L"LVM_GETCOLUMNA";
        g_mapMsgStr[LVM_GETCOLUMNW                     ] = L"LVM_GETCOLUMNW";
        g_mapMsgStr[LVM_GETCOLUMNORDERARRAY            ] = L"LVM_GETCOLUMNORDERARRAY";
        g_mapMsgStr[LVM_GETCOLUMNWIDTH                 ] = L"LVM_GETCOLUMNWIDTH";
        g_mapMsgStr[LVM_GETCOUNTPERPAGE                ] = L"LVM_GETCOUNTPERPAGE";
        g_mapMsgStr[LVM_GETEDITCONTROL                 ] = L"LVM_GETEDITCONTROL";
        g_mapMsgStr[LVM_GETEMPTYTEXT                   ] = L"LVM_GETEMPTYTEXT";
        g_mapMsgStr[LVM_GETEXTENDEDLISTVIEWSTYLE       ] = L"LVM_GETEXTENDEDLISTVIEWSTYLE";
        g_mapMsgStr[LVM_GETFOCUSEDGROUP                ] = L"LVM_GETFOCUSEDGROUP";
        g_mapMsgStr[LVM_GETFOOTERINFO                  ] = L"LVM_GETFOOTERINFO";
        g_mapMsgStr[LVM_GETFOOTERITEM                  ] = L"LVM_GETFOOTERITEM";
        g_mapMsgStr[LVM_GETFOOTERITEMRECT              ] = L"LVM_GETFOOTERITEMRECT";
        g_mapMsgStr[LVM_GETFOOTERRECT                  ] = L"LVM_GETFOOTERRECT";
        g_mapMsgStr[LVM_GETGROUPCOUNT                  ] = L"LVM_GETGROUPCOUNT";
        g_mapMsgStr[LVM_GETGROUPINFO                   ] = L"LVM_GETGROUPINFO";
        g_mapMsgStr[LVM_GETGROUPINFOBYINDEX            ] = L"LVM_GETGROUPINFOBYINDEX";
        g_mapMsgStr[LVM_GETGROUPMETRICS                ] = L"LVM_GETGROUPMETRICS";
        g_mapMsgStr[LVM_GETGROUPRECT                   ] = L"LVM_GETGROUPRECT";
        g_mapMsgStr[LVM_GETGROUPSTATE                  ] = L"LVM_GETGROUPSTATE";
        g_mapMsgStr[LVM_GETHEADER                      ] = L"LVM_GETHEADER";
        g_mapMsgStr[LVM_GETHOTCURSOR                   ] = L"LVM_GETHOTCURSOR";
        g_mapMsgStr[LVM_GETHOTITEM                     ] = L"LVM_GETHOTITEM";
        g_mapMsgStr[LVM_GETHOVERTIME                   ] = L"LVM_GETHOVERTIME";
        g_mapMsgStr[LVM_GETIMAGELIST                   ] = L"LVM_GETIMAGELIST";
        g_mapMsgStr[LVM_GETINSERTMARK                  ] = L"LVM_GETINSERTMARK";
        g_mapMsgStr[LVM_GETINSERTMARKCOLOR             ] = L"LVM_GETINSERTMARKCOLOR";
        g_mapMsgStr[LVM_GETINSERTMARKRECT              ] = L"LVM_GETINSERTMARKRECT";
        g_mapMsgStr[LVM_GETISEARCHSTRINGA              ] = L"LVM_GETISEARCHSTRINGA";
        g_mapMsgStr[LVM_GETISEARCHSTRINGW              ] = L"LVM_GETISEARCHSTRINGW";
        g_mapMsgStr[LVM_GETITEMA                       ] = L"LVM_GETITEMA";
        g_mapMsgStr[LVM_GETITEMW                       ] = L"LVM_GETITEMW";
        g_mapMsgStr[LVM_GETITEMCOUNT                   ] = L"LVM_GETITEMCOUNT";
        g_mapMsgStr[LVM_GETITEMINDEXRECT               ] = L"LVM_GETITEMINDEXRECT";
        g_mapMsgStr[LVM_GETITEMPOSITION                ] = L"LVM_GETITEMPOSITION";
        g_mapMsgStr[LVM_GETITEMRECT                    ] = L"LVM_GETITEMRECT";
        g_mapMsgStr[LVM_GETITEMSPACING                 ] = L"LVM_GETITEMSPACING";
        g_mapMsgStr[LVM_GETITEMSTATE                   ] = L"LVM_GETITEMSTATE";
        g_mapMsgStr[LVM_GETITEMTEXTA                   ] = L"LVM_GETITEMTEXTA";
        g_mapMsgStr[LVM_GETITEMTEXTW                   ] = L"LVM_GETITEMTEXTW";
        g_mapMsgStr[LVM_GETNEXTITEM                    ] = L"LVM_GETNEXTITEM";
        g_mapMsgStr[LVM_GETNEXTITEMINDEX               ] = L"LVM_GETNEXTITEMINDEX";
        g_mapMsgStr[LVM_GETNUMBEROFWORKAREAS           ] = L"LVM_GETNUMBEROFWORKAREAS";
        g_mapMsgStr[LVM_GETORIGIN                      ] = L"LVM_GETORIGIN";
        g_mapMsgStr[LVM_GETOUTLINECOLOR                ] = L"LVM_GETOUTLINECOLOR";
        g_mapMsgStr[LVM_GETSELECTEDCOLUMN              ] = L"LVM_GETSELECTEDCOLUMN";
        g_mapMsgStr[LVM_GETSELECTEDCOUNT               ] = L"LVM_GETSELECTEDCOUNT";
        g_mapMsgStr[LVM_GETSELECTIONMARK               ] = L"LVM_GETSELECTIONMARK";
        g_mapMsgStr[LVM_GETSTRINGWIDTHA                ] = L"LVM_GETSTRINGWIDTHA";
        g_mapMsgStr[LVM_GETSTRINGWIDTHW                ] = L"LVM_GETSTRINGWIDTHW";
        g_mapMsgStr[LVM_GETSUBITEMRECT                 ] = L"LVM_GETSUBITEMRECT";
        g_mapMsgStr[LVM_GETTEXTBKCOLOR                 ] = L"LVM_GETTEXTBKCOLOR";
        g_mapMsgStr[LVM_GETTEXTCOLOR                   ] = L"LVM_GETTEXTCOLOR";
        g_mapMsgStr[LVM_GETTILEINFO                    ] = L"LVM_GETTILEINFO";
        g_mapMsgStr[LVM_GETTILEVIEWINFO                ] = L"LVM_GETTILEVIEWINFO";
        g_mapMsgStr[LVM_GETTOOLTIPS                    ] = L"LVM_GETTOOLTIPS";
        g_mapMsgStr[LVM_GETTOPINDEX                    ] = L"LVM_GETTOPINDEX";
        g_mapMsgStr[LVM_GETVIEW                        ] = L"LVM_GETVIEW";
        g_mapMsgStr[LVM_GETVIEWRECT                    ] = L"LVM_GETVIEWRECT";
        g_mapMsgStr[LVM_GETWORKAREAS                   ] = L"LVM_GETWORKAREAS";
        g_mapMsgStr[LVM_HASGROUP                       ] = L"LVM_HASGROUP";
        g_mapMsgStr[LVM_HITTEST                        ] = L"LVM_HITTEST";
        g_mapMsgStr[LVM_INSERTCOLUMNA                  ] = L"LVM_INSERTCOLUMNA";
        g_mapMsgStr[LVM_INSERTCOLUMNW                  ] = L"LVM_INSERTCOLUMNW";
        g_mapMsgStr[LVM_INSERTGROUP                    ] = L"LVM_INSERTGROUP";
        g_mapMsgStr[LVM_INSERTGROUPSORTED              ] = L"LVM_INSERTGROUPSORTED";
        g_mapMsgStr[LVM_INSERTITEMA                    ] = L"LVM_INSERTITEMA";
        g_mapMsgStr[LVM_INSERTITEMW                    ] = L"LVM_INSERTITEMW";
        g_mapMsgStr[LVM_INSERTMARKHITTEST              ] = L"LVM_INSERTMARKHITTEST";
        g_mapMsgStr[LVM_ISGROUPVIEWENABLED             ] = L"LVM_ISGROUPVIEWENABLED";
        g_mapMsgStr[LVM_ISITEMVISIBLE                  ] = L"LVM_ISITEMVISIBLE";
        g_mapMsgStr[LVM_MAPIDTOINDEX                   ] = L"LVM_MAPIDTOINDEX";
        g_mapMsgStr[LVM_MAPINDEXTOID                   ] = L"LVM_MAPINDEXTOID";
        g_mapMsgStr[LVM_MOVEGROUP                      ] = L"LVM_MOVEGROUP";
        g_mapMsgStr[LVM_MOVEITEMTOGROUP                ] = L"LVM_MOVEITEMTOGROUP";
        g_mapMsgStr[LVM_REDRAWITEMS                    ] = L"LVM_REDRAWITEMS";
        g_mapMsgStr[LVM_REMOVEALLGROUPS                ] = L"LVM_REMOVEALLGROUPS";
        g_mapMsgStr[LVM_REMOVEGROUP                    ] = L"LVM_REMOVEGROUP";
        g_mapMsgStr[LVM_SCROLL                         ] = L"LVM_SCROLL";
        g_mapMsgStr[LVM_SETBKCOLOR                     ] = L"LVM_SETBKCOLOR";
        g_mapMsgStr[LVM_SETBKIMAGEA                    ] = L"LVM_SETBKIMAGEA";
        g_mapMsgStr[LVM_SETBKIMAGEW                    ] = L"LVM_SETBKIMAGEW";
        g_mapMsgStr[LVM_SETCALLBACKMASK                ] = L"LVM_SETCALLBACKMASK";
        g_mapMsgStr[LVM_SETCOLUMNA                     ] = L"LVM_SETCOLUMNA";
        g_mapMsgStr[LVM_SETCOLUMNW                     ] = L"LVM_SETCOLUMNW";
        g_mapMsgStr[LVM_SETCOLUMNORDERARRAY            ] = L"LVM_SETCOLUMNORDERARRAY";
        g_mapMsgStr[LVM_SETCOLUMNWIDTH                 ] = L"LVM_SETCOLUMNWIDTH";
        g_mapMsgStr[LVM_SETEXTENDEDLISTVIEWSTYLE       ] = L"LVM_SETEXTENDEDLISTVIEWSTYLE";
        g_mapMsgStr[LVM_SETGROUPINFO                   ] = L"LVM_SETGROUPINFO";
        g_mapMsgStr[LVM_SETGROUPMETRICS                ] = L"LVM_SETGROUPMETRICS";
        g_mapMsgStr[LVM_SETHOTCURSOR                   ] = L"LVM_SETHOTCURSOR";
        g_mapMsgStr[LVM_SETHOTITEM                     ] = L"LVM_SETHOTITEM";
        g_mapMsgStr[LVM_SETHOVERTIME                   ] = L"LVM_SETHOVERTIME";
        g_mapMsgStr[LVM_SETICONSPACING                 ] = L"LVM_SETICONSPACING";
        g_mapMsgStr[LVM_SETIMAGELIST                   ] = L"LVM_SETIMAGELIST";
        g_mapMsgStr[LVM_SETINFOTIP                     ] = L"LVM_SETINFOTIP";
        g_mapMsgStr[LVM_SETINSERTMARK                  ] = L"LVM_SETINSERTMARK";
        g_mapMsgStr[LVM_SETINSERTMARKCOLOR             ] = L"LVM_SETINSERTMARKCOLOR";
        g_mapMsgStr[LVM_SETITEMA                       ] = L"LVM_SETITEMA";
        g_mapMsgStr[LVM_SETITEMW                       ] = L"LVM_SETITEMW";
        g_mapMsgStr[LVM_SETITEMCOUNT                   ] = L"LVM_SETITEMCOUNT";
        g_mapMsgStr[LVM_SETITEMINDEXSTATE              ] = L"LVM_SETITEMINDEXSTATE";
        g_mapMsgStr[LVM_SETITEMPOSITION                ] = L"LVM_SETITEMPOSITION";
        g_mapMsgStr[LVM_SETITEMPOSITION32              ] = L"LVM_SETITEMPOSITION32";
        g_mapMsgStr[LVM_SETITEMSTATE                   ] = L"LVM_SETITEMSTATE";
        g_mapMsgStr[LVM_SETITEMTEXTA                   ] = L"LVM_SETITEMTEXTA";
        g_mapMsgStr[LVM_SETITEMTEXTW                   ] = L"LVM_SETITEMTEXTW";
        g_mapMsgStr[LVM_SETOUTLINECOLOR                ] = L"LVM_SETOUTLINECOLOR";
        g_mapMsgStr[LVM_SETSELECTEDCOLUMN              ] = L"LVM_SETSELECTEDCOLUMN";
        g_mapMsgStr[LVM_SETSELECTIONMARK               ] = L"LVM_SETSELECTIONMARK";
        g_mapMsgStr[LVM_SETTEXTBKCOLOR                 ] = L"LVM_SETTEXTBKCOLOR";
        g_mapMsgStr[LVM_SETTEXTCOLOR                   ] = L"LVM_SETTEXTCOLOR";
        g_mapMsgStr[LVM_SETTILEINFO                    ] = L"LVM_SETTILEINFO";
        g_mapMsgStr[LVM_SETTILEVIEWINFO                ] = L"LVM_SETTILEVIEWINFO";
        g_mapMsgStr[LVM_SETTOOLTIPS                    ] = L"LVM_SETTOOLTIPS";
        g_mapMsgStr[LVM_SETVIEW                        ] = L"LVM_SETVIEW";
        g_mapMsgStr[LVM_SETWORKAREAS                   ] = L"LVM_SETWORKAREAS";
        g_mapMsgStr[LVM_SORTGROUPS                     ] = L"LVM_SORTGROUPS";
        g_mapMsgStr[LVM_SORTITEMS                      ] = L"LVM_SORTITEMS";
        g_mapMsgStr[LVM_SORTITEMSEX                    ] = L"LVM_SORTITEMSEX";
        g_mapMsgStr[LVM_SUBITEMHITTEST                 ] = L"LVM_SUBITEMHITTEST";
        g_mapMsgStr[LVM_UPDATE                         ] = L"LVM_UPDATE";
        g_mapMsgStr[MCM_GETCALENDARBORDER              ] = L"MCM_GETCALENDARBORDER";
        g_mapMsgStr[MCM_GETCALENDARCOUNT               ] = L"MCM_GETCALENDARCOUNT";
        g_mapMsgStr[MCM_GETCALENDARGRIDINFO            ] = L"MCM_GETCALENDARGRIDINFO";
        g_mapMsgStr[MCM_GETCALID                       ] = L"MCM_GETCALID";
        g_mapMsgStr[MCM_GETCOLOR                       ] = L"MCM_GETCOLOR";
        g_mapMsgStr[MCM_GETCURRENTVIEW                 ] = L"MCM_GETCURRENTVIEW";
        g_mapMsgStr[MCM_GETCURSEL                      ] = L"MCM_GETCURSEL";
        g_mapMsgStr[MCM_GETFIRSTDAYOFWEEK              ] = L"MCM_GETFIRSTDAYOFWEEK";
        g_mapMsgStr[MCM_GETMAXSELCOUNT                 ] = L"MCM_GETMAXSELCOUNT";
        g_mapMsgStr[MCM_GETMAXTODAYWIDTH               ] = L"MCM_GETMAXTODAYWIDTH";
        g_mapMsgStr[MCM_GETMINREQRECT                  ] = L"MCM_GETMINREQRECT";
        g_mapMsgStr[MCM_GETMONTHDELTA                  ] = L"MCM_GETMONTHDELTA";
        g_mapMsgStr[MCM_GETMONTHRANGE                  ] = L"MCM_GETMONTHRANGE";
        g_mapMsgStr[MCM_GETRANGE                       ] = L"MCM_GETRANGE";
        g_mapMsgStr[MCM_GETSELRANGE                    ] = L"MCM_GETSELRANGE";
        g_mapMsgStr[MCM_GETTODAY                       ] = L"MCM_GETTODAY";
        g_mapMsgStr[MCM_HITTEST                        ] = L"MCM_HITTEST";
        g_mapMsgStr[MCM_SETCALENDARBORDER              ] = L"MCM_SETCALENDARBORDER";
        g_mapMsgStr[MCM_SETCALID                       ] = L"MCM_SETCALID";
        g_mapMsgStr[MCM_SETCOLOR                       ] = L"MCM_SETCOLOR";
        g_mapMsgStr[MCM_SETCURRENTVIEW                 ] = L"MCM_SETCURRENTVIEW";
        g_mapMsgStr[MCM_SETCURSEL                      ] = L"MCM_SETCURSEL";
        g_mapMsgStr[MCM_SETDAYSTATE                    ] = L"MCM_SETDAYSTATE";
        g_mapMsgStr[MCM_SETFIRSTDAYOFWEEK              ] = L"MCM_SETFIRSTDAYOFWEEK";
        g_mapMsgStr[MCM_SETMAXSELCOUNT                 ] = L"MCM_SETMAXSELCOUNT";
        g_mapMsgStr[MCM_SETMONTHDELTA                  ] = L"MCM_SETMONTHDELTA";
        g_mapMsgStr[MCM_SETRANGE                       ] = L"MCM_SETRANGE";
        g_mapMsgStr[MCM_SETSELRANGE                    ] = L"MCM_SETSELRANGE";
        g_mapMsgStr[MCM_SETTODAY                       ] = L"MCM_SETTODAY";
        g_mapMsgStr[MCM_SIZERECTTOMIN                  ] = L"MCM_SIZERECTTOMIN";
        g_mapMsgStr[NCM_DISPLAYERRORTIP                ] = L"NCM_DISPLAYERRORTIP";
        g_mapMsgStr[NCM_GETADDRESS                     ] = L"NCM_GETADDRESS";
        g_mapMsgStr[NCM_GETALLOWTYPE                   ] = L"NCM_GETALLOWTYPE";
        g_mapMsgStr[NCM_SETALLOWTYPE                   ] = L"NCM_SETALLOWTYPE";
        g_mapMsgStr[PBM_DELTAPOS                       ] = L"PBM_DELTAPOS";
        g_mapMsgStr[PBM_GETBARCOLOR                    ] = L"PBM_GETBARCOLOR";
        g_mapMsgStr[PBM_GETBKCOLOR                     ] = L"PBM_GETBKCOLOR";
        g_mapMsgStr[PBM_GETPOS                         ] = L"PBM_GETPOS";
        g_mapMsgStr[PBM_GETRANGE                       ] = L"PBM_GETRANGE";
        g_mapMsgStr[PBM_GETSTATE                       ] = L"PBM_GETSTATE";
        g_mapMsgStr[PBM_GETSTEP                        ] = L"PBM_GETSTEP";
        g_mapMsgStr[PBM_SETBARCOLOR                    ] = L"PBM_SETBARCOLOR";
        g_mapMsgStr[PBM_SETMARQUEE                     ] = L"PBM_SETMARQUEE";
        g_mapMsgStr[PBM_SETPOS                         ] = L"PBM_SETPOS";
        g_mapMsgStr[PBM_SETRANGE                       ] = L"PBM_SETRANGE";
        g_mapMsgStr[PBM_SETRANGE32                     ] = L"PBM_SETRANGE32";
        g_mapMsgStr[PBM_SETSTATE                       ] = L"PBM_SETSTATE";
        g_mapMsgStr[PBM_SETSTEP                        ] = L"PBM_SETSTEP";
        g_mapMsgStr[PBM_STEPIT                         ] = L"PBM_STEPIT";
        g_mapMsgStr[PGM_FORWARDMOUSE                   ] = L"PGM_FORWARDMOUSE";
        g_mapMsgStr[PGM_GETBKCOLOR                     ] = L"PGM_GETBKCOLOR";
        g_mapMsgStr[PGM_GETBORDER                      ] = L"PGM_GETBORDER";
        g_mapMsgStr[PGM_GETBUTTONSIZE                  ] = L"PGM_GETBUTTONSIZE";
        g_mapMsgStr[PGM_GETBUTTONSTATE                 ] = L"PGM_GETBUTTONSTATE";
        g_mapMsgStr[PGM_GETPOS                         ] = L"PGM_GETPOS";
        g_mapMsgStr[PGM_RECALCSIZE                     ] = L"PGM_RECALCSIZE";
        g_mapMsgStr[PGM_SETBKCOLOR                     ] = L"PGM_SETBKCOLOR";
        g_mapMsgStr[PGM_SETBORDER                      ] = L"PGM_SETBORDER";
        g_mapMsgStr[PGM_SETBUTTONSIZE                  ] = L"PGM_SETBUTTONSIZE";
        g_mapMsgStr[PGM_SETCHILD                       ] = L"PGM_SETCHILD";
        g_mapMsgStr[PGM_SETPOS                         ] = L"PGM_SETPOS";
        g_mapMsgStr[PGM_SETSCROLLINFO                  ] = L"PGM_SETSCROLLINFO";
        g_mapMsgStr[PSM_ADDPAGE                        ] = L"PSM_ADDPAGE";
        g_mapMsgStr[PSM_APPLY                          ] = L"PSM_APPLY";
        g_mapMsgStr[PSM_CANCELTOCLOSE                  ] = L"PSM_CANCELTOCLOSE";
        g_mapMsgStr[PSM_CHANGED                        ] = L"PSM_CHANGED";
        g_mapMsgStr[PSM_ENABLEWIZBUTTONS               ] = L"PSM_ENABLEWIZBUTTONS";
        g_mapMsgStr[PSM_GETCURRENTPAGEHWND             ] = L"PSM_GETCURRENTPAGEHWND";
        g_mapMsgStr[PSM_GETTABCONTROL                  ] = L"PSM_GETTABCONTROL";
        g_mapMsgStr[PSM_GETRESULT                      ] = L"PSM_GETRESULT";
        g_mapMsgStr[PSM_HWNDTOINDEX                    ] = L"PSM_HWNDTOINDEX";
        g_mapMsgStr[PSM_IDTOINDEX                      ] = L"PSM_IDTOINDEX";
        g_mapMsgStr[PSM_INDEXTOHWND                    ] = L"PSM_INDEXTOHWND";
        g_mapMsgStr[PSM_INDEXTOID                      ] = L"PSM_INDEXTOID";
        g_mapMsgStr[PSM_INDEXTOPAGE                    ] = L"PSM_INDEXTOPAGE";
        g_mapMsgStr[PSM_INSERTPAGE                     ] = L"PSM_INSERTPAGE";
        g_mapMsgStr[PSM_ISDIALOGMESSAGE                ] = L"PSM_ISDIALOGMESSAGE";
        g_mapMsgStr[PSM_PAGETOINDEX                    ] = L"PSM_PAGETOINDEX";
        g_mapMsgStr[PSM_PRESSBUTTON                    ] = L"PSM_PRESSBUTTON";
        g_mapMsgStr[PSM_QUERYSIBLINGS                  ] = L"PSM_QUERYSIBLINGS";
        g_mapMsgStr[PSM_REBOOTSYSTEM                   ] = L"PSM_REBOOTSYSTEM";
        g_mapMsgStr[PSM_RECALCPAGESIZES                ] = L"PSM_RECALCPAGESIZES";
        g_mapMsgStr[PSM_REMOVEPAGE                     ] = L"PSM_REMOVEPAGE";
        g_mapMsgStr[PSM_RESTARTWINDOWS                 ] = L"PSM_RESTARTWINDOWS";
        g_mapMsgStr[PSM_SETBUTTONTEXTW                 ] = L"PSM_SETBUTTONTEXTW";
        g_mapMsgStr[PSM_SETCURSEL                      ] = L"PSM_SETCURSEL";
        g_mapMsgStr[PSM_SETCURSELID                    ] = L"PSM_SETCURSELID";
        g_mapMsgStr[PSM_SETFINISHTEXTA                 ] = L"PSM_SETFINISHTEXTA";
        g_mapMsgStr[PSM_SETFINISHTEXTW                 ] = L"PSM_SETFINISHTEXTW";
        g_mapMsgStr[PSM_SETHEADERTITLEA                ] = L"PSM_SETHEADERTITLEA";
        g_mapMsgStr[PSM_SETHEADERTITLEW                ] = L"PSM_SETHEADERTITLEW";
        g_mapMsgStr[PSM_SETHEADERSUBTITLEA             ] = L"PSM_SETHEADERSUBTITLEA";
        g_mapMsgStr[PSM_SETHEADERSUBTITLEW             ] = L"PSM_SETHEADERSUBTITLEW";
        g_mapMsgStr[PSM_SETNEXTTEXTW                   ] = L"PSM_SETNEXTTEXTW";
        g_mapMsgStr[PSM_SETTITLEA                      ] = L"PSM_SETTITLEA";
        g_mapMsgStr[PSM_SETTITLEW                      ] = L"PSM_SETTITLEW";
        g_mapMsgStr[PSM_SETWIZBUTTONS                  ] = L"PSM_SETWIZBUTTONS";
        g_mapMsgStr[PSM_SHOWWIZBUTTONS                 ] = L"PSM_SHOWWIZBUTTONS";
        g_mapMsgStr[PSM_UNCHANGED                      ] = L"PSM_UNCHANGED";
        g_mapMsgStr[RB_BEGINDRAG                       ] = L"RB_BEGINDRAG";
        g_mapMsgStr[RB_DELETEBAND                      ] = L"RB_DELETEBAND";
        g_mapMsgStr[RB_DRAGMOVE                        ] = L"RB_DRAGMOVE";
        g_mapMsgStr[RB_ENDDRAG                         ] = L"RB_ENDDRAG";
        g_mapMsgStr[RB_GETBANDBORDERS                  ] = L"RB_GETBANDBORDERS";
        g_mapMsgStr[RB_GETBANDCOUNT                    ] = L"RB_GETBANDCOUNT";
        g_mapMsgStr[RB_GETBANDINFOA                    ] = L"RB_GETBANDINFOA";
        g_mapMsgStr[RB_GETBANDINFOW                    ] = L"RB_GETBANDINFOW";
        g_mapMsgStr[RB_GETBANDMARGINS                  ] = L"RB_GETBANDMARGINS";
        g_mapMsgStr[RB_GETBARHEIGHT                    ] = L"RB_GETBARHEIGHT";
        g_mapMsgStr[RB_GETBARINFO                      ] = L"RB_GETBARINFO";
        g_mapMsgStr[RB_GETBKCOLOR                      ] = L"RB_GETBKCOLOR";
        g_mapMsgStr[RB_GETEXTENDEDSTYLE                ] = L"RB_GETEXTENDEDSTYLE";
        g_mapMsgStr[RB_GETPALETTE                      ] = L"RB_GETPALETTE";
        g_mapMsgStr[RB_GETRECT                         ] = L"RB_GETRECT";
        g_mapMsgStr[RB_GETROWCOUNT                     ] = L"RB_GETROWCOUNT";
        g_mapMsgStr[RB_GETROWHEIGHT                    ] = L"RB_GETROWHEIGHT";
        g_mapMsgStr[RB_GETTEXTCOLOR                    ] = L"RB_GETTEXTCOLOR";
        g_mapMsgStr[RB_GETTOOLTIPS                     ] = L"RB_GETTOOLTIPS";
        g_mapMsgStr[RB_HITTEST                         ] = L"RB_HITTEST";
        g_mapMsgStr[RB_IDTOINDEX                       ] = L"RB_IDTOINDEX";
        g_mapMsgStr[RB_INSERTBANDA                     ] = L"RB_INSERTBANDA";
        g_mapMsgStr[RB_INSERTBANDW                     ] = L"RB_INSERTBANDW";
        g_mapMsgStr[RB_MAXIMIZEBAND                    ] = L"RB_MAXIMIZEBAND";
        g_mapMsgStr[RB_MINIMIZEBAND                    ] = L"RB_MINIMIZEBAND";
        g_mapMsgStr[RB_MOVEBAND                        ] = L"RB_MOVEBAND";
        g_mapMsgStr[RB_PUSHCHEVRON                     ] = L"RB_PUSHCHEVRON";
        g_mapMsgStr[RB_SETBANDINFOA                    ] = L"RB_SETBANDINFOA";
        g_mapMsgStr[RB_SETBANDINFOW                    ] = L"RB_SETBANDINFOW";
        g_mapMsgStr[RB_SETBANDWIDTH                    ] = L"RB_SETBANDWIDTH";
        g_mapMsgStr[RB_SETBARINFO                      ] = L"RB_SETBARINFO";
        g_mapMsgStr[RB_SETBKCOLOR                      ] = L"RB_SETBKCOLOR";
        g_mapMsgStr[RB_SETEXTENDEDSTYLE                ] = L"RB_SETEXTENDEDSTYLE";
        g_mapMsgStr[RB_SETPALETTE                      ] = L"RB_SETPALETTE";
        g_mapMsgStr[RB_SETPARENT                       ] = L"RB_SETPARENT";
        g_mapMsgStr[RB_SETTEXTCOLOR                    ] = L"RB_SETTEXTCOLOR";
        g_mapMsgStr[RB_SETTOOLTIPS                     ] = L"RB_SETTOOLTIPS";
        g_mapMsgStr[RB_SHOWBAND                        ] = L"RB_SHOWBAND";
        g_mapMsgStr[RB_SIZETORECT                      ] = L"RB_SIZETORECT";
        g_mapMsgStr[SB_GETBORDERS                      ] = L"SB_GETBORDERS";
        g_mapMsgStr[SB_GETICON                         ] = L"SB_GETICON";
        g_mapMsgStr[SB_GETPARTS                        ] = L"SB_GETPARTS";
        g_mapMsgStr[SB_GETRECT                         ] = L"SB_GETRECT";
        g_mapMsgStr[SB_GETTEXTA                        ] = L"SB_GETTEXTA";
        g_mapMsgStr[SB_GETTEXTW                        ] = L"SB_GETTEXTW";
        g_mapMsgStr[SB_GETTEXTLENGTHA                  ] = L"SB_GETTEXTLENGTHA";
        g_mapMsgStr[SB_GETTEXTLENGTHW                  ] = L"SB_GETTEXTLENGTHW";
        g_mapMsgStr[SB_GETTIPTEXTA                     ] = L"SB_GETTIPTEXTA";
        g_mapMsgStr[SB_GETTIPTEXTW                     ] = L"SB_GETTIPTEXTW";
        g_mapMsgStr[SB_ISSIMPLE                        ] = L"SB_ISSIMPLE";
        g_mapMsgStr[SB_SETICON                         ] = L"SB_SETICON";
        g_mapMsgStr[SB_SETMINHEIGHT                    ] = L"SB_SETMINHEIGHT";
        g_mapMsgStr[SB_SETPARTS                        ] = L"SB_SETPARTS";
        g_mapMsgStr[SB_SETTEXTA                        ] = L"SB_SETTEXTA";
        g_mapMsgStr[SB_SETTEXTW                        ] = L"SB_SETTEXTW";
        g_mapMsgStr[SB_SETTIPTEXTA                     ] = L"SB_SETTIPTEXTA";
        g_mapMsgStr[SB_SETTIPTEXTW                     ] = L"SB_SETTIPTEXTW";
        g_mapMsgStr[SB_SIMPLE                          ] = L"SB_SIMPLE";
        g_mapMsgStr[SBM_ENABLE_ARROWS                  ] = L"SBM_ENABLE_ARROWS";
        g_mapMsgStr[SBM_GETPOS                         ] = L"SBM_GETPOS";
        g_mapMsgStr[SBM_GETRANGE                       ] = L"SBM_GETRANGE";
        g_mapMsgStr[SBM_GETSCROLLBARINFO               ] = L"SBM_GETSCROLLBARINFO";
        g_mapMsgStr[SBM_GETSCROLLINFO                  ] = L"SBM_GETSCROLLINFO";
        g_mapMsgStr[SBM_SETPOS                         ] = L"SBM_SETPOS";
        g_mapMsgStr[SBM_SETRANGE                       ] = L"SBM_SETRANGE";
        g_mapMsgStr[SBM_SETRANGEREDRAW                 ] = L"SBM_SETRANGEREDRAW";
        g_mapMsgStr[SBM_SETSCROLLINFO                  ] = L"SBM_SETSCROLLINFO";
        g_mapMsgStr[STM_GETICON                        ] = L"STM_GETICON";
        g_mapMsgStr[STM_GETIMAGE                       ] = L"STM_GETIMAGE";
        g_mapMsgStr[STM_SETICON                        ] = L"STM_SETICON";
        g_mapMsgStr[STM_SETIMAGE                       ] = L"STM_SETIMAGE";
        g_mapMsgStr[TB_ADDBITMAP                       ] = L"TB_ADDBITMAP";
        g_mapMsgStr[TB_ADDBUTTONSA                     ] = L"TB_ADDBUTTONSA";
        g_mapMsgStr[TB_ADDBUTTONSW                     ] = L"TB_ADDBUTTONSW";
        g_mapMsgStr[TB_ADDSTRINGA                      ] = L"TB_ADDSTRINGA";
        g_mapMsgStr[TB_ADDSTRINGW                      ] = L"TB_ADDSTRINGW";
        g_mapMsgStr[TB_AUTOSIZE                        ] = L"TB_AUTOSIZE";
        g_mapMsgStr[TB_BUTTONCOUNT                     ] = L"TB_BUTTONCOUNT";
        g_mapMsgStr[TB_BUTTONSTRUCTSIZE                ] = L"TB_BUTTONSTRUCTSIZE";
        g_mapMsgStr[TB_CHANGEBITMAP                    ] = L"TB_CHANGEBITMAP";
        g_mapMsgStr[TB_CHECKBUTTON                     ] = L"TB_CHECKBUTTON";
        g_mapMsgStr[TB_COMMANDTOINDEX                  ] = L"TB_COMMANDTOINDEX";
        g_mapMsgStr[TB_CUSTOMIZE                       ] = L"TB_CUSTOMIZE";
        g_mapMsgStr[TB_DELETEBUTTON                    ] = L"TB_DELETEBUTTON";
        g_mapMsgStr[TB_ENABLEBUTTON                    ] = L"TB_ENABLEBUTTON";
        g_mapMsgStr[TB_GETANCHORHIGHLIGHT              ] = L"TB_GETANCHORHIGHLIGHT";
        g_mapMsgStr[TB_GETBITMAP                       ] = L"TB_GETBITMAP";
        g_mapMsgStr[TB_GETBITMAPFLAGS                  ] = L"TB_GETBITMAPFLAGS";
        g_mapMsgStr[TB_GETBUTTON                       ] = L"TB_GETBUTTON";
        g_mapMsgStr[TB_GETBUTTONINFOA                  ] = L"TB_GETBUTTONINFOA";
        g_mapMsgStr[TB_GETBUTTONINFOW                  ] = L"TB_GETBUTTONINFOW";
        g_mapMsgStr[TB_GETBUTTONSIZE                   ] = L"TB_GETBUTTONSIZE";
        g_mapMsgStr[TB_GETBUTTONTEXTA                  ] = L"TB_GETBUTTONTEXTA";
        g_mapMsgStr[TB_GETBUTTONTEXTW                  ] = L"TB_GETBUTTONTEXTW";
        g_mapMsgStr[TB_GETDISABLEDIMAGELIST            ] = L"TB_GETDISABLEDIMAGELIST";
        g_mapMsgStr[TB_GETEXTENDEDSTYLE                ] = L"TB_GETEXTENDEDSTYLE";
        g_mapMsgStr[TB_GETHOTIMAGELIST                 ] = L"TB_GETHOTIMAGELIST";
        g_mapMsgStr[TB_GETHOTITEM                      ] = L"TB_GETHOTITEM";
        g_mapMsgStr[TB_GETIDEALSIZE                    ] = L"TB_GETIDEALSIZE";
        g_mapMsgStr[TB_GETIMAGELIST                    ] = L"TB_GETIMAGELIST";
        g_mapMsgStr[TB_GETIMAGELISTCOUNT               ] = L"TB_GETIMAGELISTCOUNT";
        g_mapMsgStr[TB_GETINSERTMARK                   ] = L"TB_GETINSERTMARK";
        g_mapMsgStr[TB_GETINSERTMARKCOLOR              ] = L"TB_GETINSERTMARKCOLOR";
        g_mapMsgStr[TB_GETITEMDROPDOWNRECT             ] = L"TB_GETITEMDROPDOWNRECT";
        g_mapMsgStr[TB_GETITEMRECT                     ] = L"TB_GETITEMRECT";
        g_mapMsgStr[TB_GETMAXSIZE                      ] = L"TB_GETMAXSIZE";
        g_mapMsgStr[TB_GETMETRICS                      ] = L"TB_GETMETRICS";
        g_mapMsgStr[TB_GETOBJECT                       ] = L"TB_GETOBJECT";
        g_mapMsgStr[TB_GETPADDING                      ] = L"TB_GETPADDING";
        g_mapMsgStr[TB_GETPRESSEDIMAGELIST             ] = L"TB_GETPRESSEDIMAGELIST";
        g_mapMsgStr[TB_GETRECT                         ] = L"TB_GETRECT";
        g_mapMsgStr[TB_GETROWS                         ] = L"TB_GETROWS";
        g_mapMsgStr[TB_GETSTATE                        ] = L"TB_GETSTATE";
        g_mapMsgStr[TB_GETSTRINGA                      ] = L"TB_GETSTRINGA";
        g_mapMsgStr[TB_GETSTRINGW                      ] = L"TB_GETSTRINGW";
        g_mapMsgStr[TB_GETSTYLE                        ] = L"TB_GETSTYLE";
        g_mapMsgStr[TB_GETTEXTROWS                     ] = L"TB_GETTEXTROWS";
        g_mapMsgStr[TB_GETTOOLTIPS                     ] = L"TB_GETTOOLTIPS";
        g_mapMsgStr[TB_HASACCELERATOR                  ] = L"TB_HASACCELERATOR";
        g_mapMsgStr[TB_HIDEBUTTON                      ] = L"TB_HIDEBUTTON";
        g_mapMsgStr[TB_HITTEST                         ] = L"TB_HITTEST";
        g_mapMsgStr[TB_INDETERMINATE                   ] = L"TB_INDETERMINATE";
        g_mapMsgStr[TB_INSERTBUTTONA                   ] = L"TB_INSERTBUTTONA";
        g_mapMsgStr[TB_INSERTBUTTONW                   ] = L"TB_INSERTBUTTONW";
        g_mapMsgStr[TB_INSERTMARKHITTEST               ] = L"TB_INSERTMARKHITTEST";
        g_mapMsgStr[TB_ISBUTTONCHECKED                 ] = L"TB_ISBUTTONCHECKED";
        g_mapMsgStr[TB_ISBUTTONENABLED                 ] = L"TB_ISBUTTONENABLED";
        g_mapMsgStr[TB_ISBUTTONHIDDEN                  ] = L"TB_ISBUTTONHIDDEN";
        g_mapMsgStr[TB_ISBUTTONHIGHLIGHTED             ] = L"TB_ISBUTTONHIGHLIGHTED";
        g_mapMsgStr[TB_ISBUTTONINDETERMINATE           ] = L"TB_ISBUTTONINDETERMINATE";
        g_mapMsgStr[TB_ISBUTTONPRESSED                 ] = L"TB_ISBUTTONPRESSED";
        g_mapMsgStr[TB_LOADIMAGES                      ] = L"TB_LOADIMAGES";
        g_mapMsgStr[TB_MAPACCELERATORA                 ] = L"TB_MAPACCELERATORA";
        g_mapMsgStr[TB_MAPACCELERATORW                 ] = L"TB_MAPACCELERATORW";
        g_mapMsgStr[TB_MARKBUTTON                      ] = L"TB_MARKBUTTON";
        g_mapMsgStr[TB_MOVEBUTTON                      ] = L"TB_MOVEBUTTON";
        g_mapMsgStr[TB_PRESSBUTTON                     ] = L"TB_PRESSBUTTON";
        g_mapMsgStr[TB_REPLACEBITMAP                   ] = L"TB_REPLACEBITMAP";
        g_mapMsgStr[TB_SAVERESTOREA                    ] = L"TB_SAVERESTOREA";
        g_mapMsgStr[TB_SAVERESTOREW                    ] = L"TB_SAVERESTOREW";
        g_mapMsgStr[TB_SETANCHORHIGHLIGHT              ] = L"TB_SETANCHORHIGHLIGHT";
        g_mapMsgStr[TB_SETBITMAPSIZE                   ] = L"TB_SETBITMAPSIZE";
        g_mapMsgStr[TB_SETBOUNDINGSIZE                 ] = L"TB_SETBOUNDINGSIZE";
        g_mapMsgStr[TB_SETBUTTONINFOA                  ] = L"TB_SETBUTTONINFOA";
        g_mapMsgStr[TB_SETBUTTONINFOW                  ] = L"TB_SETBUTTONINFOW";
        g_mapMsgStr[TB_SETBUTTONSIZE                   ] = L"TB_SETBUTTONSIZE";
        g_mapMsgStr[TB_SETBUTTONWIDTH                  ] = L"TB_SETBUTTONWIDTH";
        g_mapMsgStr[TB_SETCMDID                        ] = L"TB_SETCMDID";
        g_mapMsgStr[TB_SETDISABLEDIMAGELIST            ] = L"TB_SETDISABLEDIMAGELIST";
        g_mapMsgStr[TB_SETDRAWTEXTFLAGS                ] = L"TB_SETDRAWTEXTFLAGS";
        g_mapMsgStr[TB_SETEXTENDEDSTYLE                ] = L"TB_SETEXTENDEDSTYLE";
        g_mapMsgStr[TB_SETHOTIMAGELIST                 ] = L"TB_SETHOTIMAGELIST";
        g_mapMsgStr[TB_SETHOTITEM                      ] = L"TB_SETHOTITEM";
        g_mapMsgStr[TB_SETHOTITEM2                     ] = L"TB_SETHOTITEM2";
        g_mapMsgStr[TB_SETIMAGELIST                    ] = L"TB_SETIMAGELIST";
        g_mapMsgStr[TB_SETINDENT                       ] = L"TB_SETINDENT";
        g_mapMsgStr[TB_SETINSERTMARK                   ] = L"TB_SETINSERTMARK";
        g_mapMsgStr[TB_SETINSERTMARKCOLOR              ] = L"TB_SETINSERTMARKCOLOR";
        g_mapMsgStr[TB_SETLISTGAP                      ] = L"TB_SETLISTGAP";
        g_mapMsgStr[TB_SETMAXTEXTROWS                  ] = L"TB_SETMAXTEXTROWS";
        g_mapMsgStr[TB_SETMETRICS                      ] = L"TB_SETMETRICS";
        g_mapMsgStr[TB_SETPADDING                      ] = L"TB_SETPADDING";
        g_mapMsgStr[TB_SETPARENT                       ] = L"TB_SETPARENT";
        g_mapMsgStr[TB_SETPRESSEDIMAGELIST             ] = L"TB_SETPRESSEDIMAGELIST";
        g_mapMsgStr[TB_SETROWS                         ] = L"TB_SETROWS";
        g_mapMsgStr[TB_SETSTATE                        ] = L"TB_SETSTATE";
        g_mapMsgStr[TB_SETSTYLE                        ] = L"TB_SETSTYLE";
        g_mapMsgStr[TB_SETTOOLTIPS                     ] = L"TB_SETTOOLTIPS";
        g_mapMsgStr[TBM_CLEARSEL                       ] = L"TBM_CLEARSEL";
        g_mapMsgStr[TBM_CLEARTICS                      ] = L"TBM_CLEARTICS";
        g_mapMsgStr[TBM_GETBUDDY                       ] = L"TBM_GETBUDDY";
        g_mapMsgStr[TBM_GETCHANNELRECT                 ] = L"TBM_GETCHANNELRECT";
        g_mapMsgStr[TBM_GETLINESIZE                    ] = L"TBM_GETLINESIZE";
        g_mapMsgStr[TBM_GETNUMTICS                     ] = L"TBM_GETNUMTICS";
        g_mapMsgStr[TBM_GETPAGESIZE                    ] = L"TBM_GETPAGESIZE";
        g_mapMsgStr[TBM_GETPOS                         ] = L"TBM_GETPOS";
        g_mapMsgStr[TBM_GETPTICS                       ] = L"TBM_GETPTICS";
        g_mapMsgStr[TBM_GETRANGEMAX                    ] = L"TBM_GETRANGEMAX";
        g_mapMsgStr[TBM_GETRANGEMIN                    ] = L"TBM_GETRANGEMIN";
        g_mapMsgStr[TBM_GETSELEND                      ] = L"TBM_GETSELEND";
        g_mapMsgStr[TBM_GETSELSTART                    ] = L"TBM_GETSELSTART";
        g_mapMsgStr[TBM_GETTHUMBLENGTH                 ] = L"TBM_GETTHUMBLENGTH";
        g_mapMsgStr[TBM_GETTHUMBRECT                   ] = L"TBM_GETTHUMBRECT";
        g_mapMsgStr[TBM_GETTIC                         ] = L"TBM_GETTIC";
        g_mapMsgStr[TBM_GETTICPOS                      ] = L"TBM_GETTICPOS";
        g_mapMsgStr[TBM_GETTOOLTIPS                    ] = L"TBM_GETTOOLTIPS";
        g_mapMsgStr[TBM_SETBUDDY                       ] = L"TBM_SETBUDDY";
        g_mapMsgStr[TBM_SETLINESIZE                    ] = L"TBM_SETLINESIZE";
        g_mapMsgStr[TBM_SETPAGESIZE                    ] = L"TBM_SETPAGESIZE";
        g_mapMsgStr[TBM_SETPOS                         ] = L"TBM_SETPOS";
        g_mapMsgStr[TBM_SETPOSNOTIFY                   ] = L"TBM_SETPOSNOTIFY";
        g_mapMsgStr[TBM_SETRANGE                       ] = L"TBM_SETRANGE";
        g_mapMsgStr[TBM_SETRANGEMAX                    ] = L"TBM_SETRANGEMAX";
        g_mapMsgStr[TBM_SETRANGEMIN                    ] = L"TBM_SETRANGEMIN";
        g_mapMsgStr[TBM_SETSEL                         ] = L"TBM_SETSEL";
        g_mapMsgStr[TBM_SETSELEND                      ] = L"TBM_SETSELEND";
        g_mapMsgStr[TBM_SETSELSTART                    ] = L"TBM_SETSELSTART";
        g_mapMsgStr[TBM_SETTHUMBLENGTH                 ] = L"TBM_SETTHUMBLENGTH";
        g_mapMsgStr[TBM_SETTIC                         ] = L"TBM_SETTIC";
        g_mapMsgStr[TBM_SETTICFREQ                     ] = L"TBM_SETTICFREQ";
        g_mapMsgStr[TBM_SETTIPSIDE                     ] = L"TBM_SETTIPSIDE";
        g_mapMsgStr[TBM_SETTOOLTIPS                    ] = L"TBM_SETTOOLTIPS";
        g_mapMsgStr[TCM_ADJUSTRECT                     ] = L"TCM_ADJUSTRECT";
        g_mapMsgStr[TCM_DELETEALLITEMS                 ] = L"TCM_DELETEALLITEMS";
        g_mapMsgStr[TCM_DELETEITEM                     ] = L"TCM_DELETEITEM";
        g_mapMsgStr[TCM_DESELECTALL                    ] = L"TCM_DESELECTALL";
        g_mapMsgStr[TCM_GETCURFOCUS                    ] = L"TCM_GETCURFOCUS";
        g_mapMsgStr[TCM_GETCURSEL                      ] = L"TCM_GETCURSEL";
        g_mapMsgStr[TCM_GETEXTENDEDSTYLE               ] = L"TCM_GETEXTENDEDSTYLE";
        g_mapMsgStr[TCM_GETIMAGELIST                   ] = L"TCM_GETIMAGELIST";
        g_mapMsgStr[TCM_GETITEMA                       ] = L"TCM_GETITEMA";
        g_mapMsgStr[TCM_GETITEMW                       ] = L"TCM_GETITEMW";
        g_mapMsgStr[TCM_GETITEMCOUNT                   ] = L"TCM_GETITEMCOUNT";
        g_mapMsgStr[TCM_GETITEMRECT                    ] = L"TCM_GETITEMRECT";
        g_mapMsgStr[TCM_GETROWCOUNT                    ] = L"TCM_GETROWCOUNT";
        g_mapMsgStr[TCM_GETTOOLTIPS                    ] = L"TCM_GETTOOLTIPS";
        g_mapMsgStr[TCM_HIGHLIGHTITEM                  ] = L"TCM_HIGHLIGHTITEM";
        g_mapMsgStr[TCM_HITTEST                        ] = L"TCM_HITTEST";
        g_mapMsgStr[TCM_INSERTITEMA                    ] = L"TCM_INSERTITEMA";
        g_mapMsgStr[TCM_INSERTITEMW                    ] = L"TCM_INSERTITEMW";
        g_mapMsgStr[TCM_REMOVEIMAGE                    ] = L"TCM_REMOVEIMAGE";
        g_mapMsgStr[TCM_SETCURFOCUS                    ] = L"TCM_SETCURFOCUS";
        g_mapMsgStr[TCM_SETCURSEL                      ] = L"TCM_SETCURSEL";
        g_mapMsgStr[TCM_SETEXTENDEDSTYLE               ] = L"TCM_SETEXTENDEDSTYLE";
        g_mapMsgStr[TCM_SETIMAGELIST                   ] = L"TCM_SETIMAGELIST";
        g_mapMsgStr[TCM_SETITEMA                       ] = L"TCM_SETITEMA";
        g_mapMsgStr[TCM_SETITEMW                       ] = L"TCM_SETITEMW";
        g_mapMsgStr[TCM_SETITEMEXTRA                   ] = L"TCM_SETITEMEXTRA";
        g_mapMsgStr[TCM_SETITEMSIZE                    ] = L"TCM_SETITEMSIZE";
        g_mapMsgStr[TCM_SETMINTABWIDTH                 ] = L"TCM_SETMINTABWIDTH";
        g_mapMsgStr[TCM_SETPADDING                     ] = L"TCM_SETPADDING";
        g_mapMsgStr[TCM_SETTOOLTIPS                    ] = L"TCM_SETTOOLTIPS";
        g_mapMsgStr[TTM_ACTIVATE                       ] = L"TTM_ACTIVATE";
        g_mapMsgStr[TTM_ADDTOOLA                       ] = L"TTM_ADDTOOLA";
        g_mapMsgStr[TTM_ADDTOOLW                       ] = L"TTM_ADDTOOLW";
        g_mapMsgStr[TTM_ADJUSTRECT                     ] = L"TTM_ADJUSTRECT";
        g_mapMsgStr[TTM_DELTOOLA                       ] = L"TTM_DELTOOLA";
        g_mapMsgStr[TTM_DELTOOLW                       ] = L"TTM_DELTOOLW";
        g_mapMsgStr[TTM_ENUMTOOLSA                     ] = L"TTM_ENUMTOOLSA";
        g_mapMsgStr[TTM_ENUMTOOLSW                     ] = L"TTM_ENUMTOOLSW";
        g_mapMsgStr[TTM_GETBUBBLESIZE                  ] = L"TTM_GETBUBBLESIZE";
        g_mapMsgStr[TTM_GETCURRENTTOOLA                ] = L"TTM_GETCURRENTTOOLA";
        g_mapMsgStr[TTM_GETCURRENTTOOLW                ] = L"TTM_GETCURRENTTOOLW";
        g_mapMsgStr[TTM_GETDELAYTIME                   ] = L"TTM_GETDELAYTIME";
        g_mapMsgStr[TTM_GETMARGIN                      ] = L"TTM_GETMARGIN";
        g_mapMsgStr[TTM_GETMAXTIPWIDTH                 ] = L"TTM_GETMAXTIPWIDTH";
        g_mapMsgStr[TTM_GETTEXTA                       ] = L"TTM_GETTEXTA";
        g_mapMsgStr[TTM_GETTEXTW                       ] = L"TTM_GETTEXTW";
        g_mapMsgStr[TTM_GETTIPBKCOLOR                  ] = L"TTM_GETTIPBKCOLOR";
        g_mapMsgStr[TTM_GETTIPTEXTCOLOR                ] = L"TTM_GETTIPTEXTCOLOR";
        g_mapMsgStr[TTM_GETTITLE                       ] = L"TTM_GETTITLE";
        g_mapMsgStr[TTM_GETTOOLCOUNT                   ] = L"TTM_GETTOOLCOUNT";
        g_mapMsgStr[TTM_GETTOOLINFOA                   ] = L"TTM_GETTOOLINFOA";
        g_mapMsgStr[TTM_GETTOOLINFOW                   ] = L"TTM_GETTOOLINFOW";
        g_mapMsgStr[TTM_HITTESTA                       ] = L"TTM_HITTESTA";
        g_mapMsgStr[TTM_HITTESTW                       ] = L"TTM_HITTESTW";
        g_mapMsgStr[TTM_NEWTOOLRECTA                   ] = L"TTM_NEWTOOLRECTA";
        g_mapMsgStr[TTM_NEWTOOLRECTW                   ] = L"TTM_NEWTOOLRECTW";
        g_mapMsgStr[TTM_POP                            ] = L"TTM_POP";
        g_mapMsgStr[TTM_POPUP                          ] = L"TTM_POPUP";
        g_mapMsgStr[TTM_RELAYEVENT                     ] = L"TTM_RELAYEVENT";
        g_mapMsgStr[TTM_SETDELAYTIME                   ] = L"TTM_SETDELAYTIME";
        g_mapMsgStr[TTM_SETMARGIN                      ] = L"TTM_SETMARGIN";
        g_mapMsgStr[TTM_SETMAXTIPWIDTH                 ] = L"TTM_SETMAXTIPWIDTH";
        g_mapMsgStr[TTM_SETTIPBKCOLOR                  ] = L"TTM_SETTIPBKCOLOR";
        g_mapMsgStr[TTM_SETTIPTEXTCOLOR                ] = L"TTM_SETTIPTEXTCOLOR";
        g_mapMsgStr[TTM_SETTITLEA                      ] = L"TTM_SETTITLEA";
        g_mapMsgStr[TTM_SETTITLEW                      ] = L"TTM_SETTITLEW";
        g_mapMsgStr[TTM_SETTOOLINFOA                   ] = L"TTM_SETTOOLINFOA";
        g_mapMsgStr[TTM_SETTOOLINFOW                   ] = L"TTM_SETTOOLINFOW";
        g_mapMsgStr[TTM_TRACKACTIVATE                  ] = L"TTM_TRACKACTIVATE";
        g_mapMsgStr[TTM_TRACKPOSITION                  ] = L"TTM_TRACKPOSITION";
        g_mapMsgStr[TTM_UPDATE                         ] = L"TTM_UPDATE";
        g_mapMsgStr[TTM_UPDATETIPTEXTA                 ] = L"TTM_UPDATETIPTEXTA";
        g_mapMsgStr[TTM_UPDATETIPTEXTW                 ] = L"TTM_UPDATETIPTEXTW";
        g_mapMsgStr[TTM_WINDOWFROMPOINT                ] = L"TTM_WINDOWFROMPOINT";
        g_mapMsgStr[TVM_CREATEDRAGIMAGE                ] = L"TVM_CREATEDRAGIMAGE";
        g_mapMsgStr[TVM_DELETEITEM                     ] = L"TVM_DELETEITEM";
        g_mapMsgStr[TVM_EDITLABELA                     ] = L"TVM_EDITLABELA";
        g_mapMsgStr[TVM_EDITLABELW                     ] = L"TVM_EDITLABELW";
        g_mapMsgStr[TVM_ENDEDITLABELNOW                ] = L"TVM_ENDEDITLABELNOW";
        g_mapMsgStr[TVM_ENSUREVISIBLE                  ] = L"TVM_ENSUREVISIBLE";
        g_mapMsgStr[TVM_EXPAND                         ] = L"TVM_EXPAND";
        g_mapMsgStr[TVM_GETBKCOLOR                     ] = L"TVM_GETBKCOLOR";
        g_mapMsgStr[TVM_GETCOUNT                       ] = L"TVM_GETCOUNT";
        g_mapMsgStr[TVM_GETEDITCONTROL                 ] = L"TVM_GETEDITCONTROL";
        g_mapMsgStr[TVM_GETEXTENDEDSTYLE               ] = L"TVM_GETEXTENDEDSTYLE";
        g_mapMsgStr[TVM_GETIMAGELIST                   ] = L"TVM_GETIMAGELIST";
        g_mapMsgStr[TVM_GETINDENT                      ] = L"TVM_GETINDENT";
        g_mapMsgStr[TVM_GETINSERTMARKCOLOR             ] = L"TVM_GETINSERTMARKCOLOR";
        g_mapMsgStr[TVM_GETISEARCHSTRINGA              ] = L"TVM_GETISEARCHSTRINGA";
        g_mapMsgStr[TVM_GETISEARCHSTRINGW              ] = L"TVM_GETISEARCHSTRINGW";
        g_mapMsgStr[TVM_GETITEMA                       ] = L"TVM_GETITEMA";
        g_mapMsgStr[TVM_GETITEMW                       ] = L"TVM_GETITEMW";
        g_mapMsgStr[TVM_GETITEMHEIGHT                  ] = L"TVM_GETITEMHEIGHT";
        g_mapMsgStr[TVM_GETITEMPARTRECT                ] = L"TVM_GETITEMPARTRECT";
        g_mapMsgStr[TVM_GETITEMRECT                    ] = L"TVM_GETITEMRECT";
        g_mapMsgStr[TVM_GETITEMSTATE                   ] = L"TVM_GETITEMSTATE";
        g_mapMsgStr[TVM_GETLINECOLOR                   ] = L"TVM_GETLINECOLOR";
        g_mapMsgStr[TVM_GETNEXTITEM                    ] = L"TVM_GETNEXTITEM";
        g_mapMsgStr[TVM_GETSCROLLTIME                  ] = L"TVM_GETSCROLLTIME";
        g_mapMsgStr[TVM_GETSELECTEDCOUNT               ] = L"TVM_GETSELECTEDCOUNT";
        g_mapMsgStr[TVM_GETTEXTCOLOR                   ] = L"TVM_GETTEXTCOLOR";
        g_mapMsgStr[TVM_GETTOOLTIPS                    ] = L"TVM_GETTOOLTIPS";
        g_mapMsgStr[TVM_GETVISIBLECOUNT                ] = L"TVM_GETVISIBLECOUNT";
        g_mapMsgStr[TVM_HITTEST                        ] = L"TVM_HITTEST";
        g_mapMsgStr[TVM_INSERTITEMA                    ] = L"TVM_INSERTITEMA";
        g_mapMsgStr[TVM_INSERTITEMW                    ] = L"TVM_INSERTITEMW";
        g_mapMsgStr[TVM_MAPACCIDTOHTREEITEM            ] = L"TVM_MAPACCIDTOHTREEITEM";
        g_mapMsgStr[TVM_MAPHTREEITEMTOACCID            ] = L"TVM_MAPHTREEITEMTOACCID";
        g_mapMsgStr[TVM_SELECTITEM                     ] = L"TVM_SELECTITEM";
        g_mapMsgStr[TVM_SETAUTOSCROLLINFO              ] = L"TVM_SETAUTOSCROLLINFO";
        g_mapMsgStr[TVM_SETBKCOLOR                     ] = L"TVM_SETBKCOLOR";
        g_mapMsgStr[TVM_SETBORDER                      ] = L"TVM_SETBORDER";
        g_mapMsgStr[TVM_SETEXTENDEDSTYLE               ] = L"TVM_SETEXTENDEDSTYLE";
        g_mapMsgStr[TVM_SETHOT                         ] = L"TVM_SETHOT";
        g_mapMsgStr[TVM_SETIMAGELIST                   ] = L"TVM_SETIMAGELIST";
        g_mapMsgStr[TVM_SETINDENT                      ] = L"TVM_SETINDENT";
        g_mapMsgStr[TVM_SETINSERTMARK                  ] = L"TVM_SETINSERTMARK";
        g_mapMsgStr[TVM_SETINSERTMARKCOLOR             ] = L"TVM_SETINSERTMARKCOLOR";
        g_mapMsgStr[TVM_SETITEMA                       ] = L"TVM_SETITEMA";
        g_mapMsgStr[TVM_SETITEMW                       ] = L"TVM_SETITEMW";
        g_mapMsgStr[TVM_SETITEMHEIGHT                  ] = L"TVM_SETITEMHEIGHT";
        g_mapMsgStr[TVM_SETLINECOLOR                   ] = L"TVM_SETLINECOLOR";
        g_mapMsgStr[TVM_SETSCROLLTIME                  ] = L"TVM_SETSCROLLTIME";
        g_mapMsgStr[TVM_SETTEXTCOLOR                   ] = L"TVM_SETTEXTCOLOR";
        g_mapMsgStr[TVM_SETTOOLTIPS                    ] = L"TVM_SETTOOLTIPS";
        g_mapMsgStr[TVM_SHOWINFOTIP                    ] = L"TVM_SHOWINFOTIP";
        g_mapMsgStr[TVM_SORTCHILDREN                   ] = L"TVM_SORTCHILDREN";
        g_mapMsgStr[TVM_SORTCHILDRENCB                 ] = L"TVM_SORTCHILDRENCB";
        g_mapMsgStr[UDM_GETACCEL                       ] = L"UDM_GETACCEL";
        g_mapMsgStr[UDM_GETBASE                        ] = L"UDM_GETBASE";
        g_mapMsgStr[UDM_GETBUDDY                       ] = L"UDM_GETBUDDY";
        g_mapMsgStr[UDM_GETPOS                         ] = L"UDM_GETPOS";
        g_mapMsgStr[UDM_GETPOS32                       ] = L"UDM_GETPOS32";
        g_mapMsgStr[UDM_GETRANGE                       ] = L"UDM_GETRANGE";
        g_mapMsgStr[UDM_GETRANGE32                     ] = L"UDM_GETRANGE32";
        g_mapMsgStr[UDM_SETACCEL                       ] = L"UDM_SETACCEL";
        g_mapMsgStr[UDM_SETBASE                        ] = L"UDM_SETBASE";
        g_mapMsgStr[UDM_SETBUDDY                       ] = L"UDM_SETBUDDY";
        g_mapMsgStr[UDM_SETPOS                         ] = L"UDM_SETPOS";
        g_mapMsgStr[UDM_SETPOS32                       ] = L"UDM_SETPOS32";
        g_mapMsgStr[UDM_SETRANGE                       ] = L"UDM_SETRANGE";
        g_mapMsgStr[UDM_SETRANGE32                     ] = L"UDM_SETRANGE32";
        g_mapMsgStr[WM_ACTIVATE                        ] = L"WM_ACTIVATE";
        g_mapMsgStr[WM_ACTIVATEAPP                     ] = L"WM_ACTIVATEAPP";
        g_mapMsgStr[WM_ACTIVATETOPLEVEL                ] = L"WM_ACTIVATETOPLEVEL";
        g_mapMsgStr[WM_APPCOMMAND                      ] = L"WM_APPCOMMAND";
        g_mapMsgStr[WM_ASKCBFORMATNAME                 ] = L"WM_ASKCBFORMATNAME";
        g_mapMsgStr[WM_CANCELJOURNAL                   ] = L"WM_CANCELJOURNAL";
        g_mapMsgStr[WM_CANCELMODE                      ] = L"WM_CANCELMODE";
        g_mapMsgStr[WM_CAPTURECHANGED                  ] = L"WM_CAPTURECHANGED";
        g_mapMsgStr[WM_CHANGECBCHAIN                   ] = L"WM_CHANGECBCHAIN";
        g_mapMsgStr[WM_CHANGEUISTATE                   ] = L"WM_CHANGEUISTATE";
        g_mapMsgStr[WM_CHAR                            ] = L"WM_CHAR";
        g_mapMsgStr[WM_CHARTOITEM                      ] = L"WM_CHARTOITEM";
        g_mapMsgStr[WM_CHILDACTIVATE                   ] = L"WM_CHILDACTIVATE";
        g_mapMsgStr[WM_CHOOSEFONT_GETLOGFONT           ] = L"WM_CHOOSEFONT_GETLOGFONT";
        g_mapMsgStr[WM_CHOOSEFONT_SETFLAGS             ] = L"WM_CHOOSEFONT_SETFLAGS";
        g_mapMsgStr[WM_CHOOSEFONT_SETLOGFONT           ] = L"WM_CHOOSEFONT_SETLOGFONT";
        g_mapMsgStr[WM_CLEAR                           ] = L"WM_CLEAR";
        g_mapMsgStr[WM_CLIPBOARDUPDATE                 ] = L"WM_CLIPBOARDUPDATE";
        g_mapMsgStr[WM_CLOSE                           ] = L"WM_CLOSE";
        g_mapMsgStr[WM_COMMAND                         ] = L"WM_COMMAND";
        g_mapMsgStr[WM_COMMANDHELP                     ] = L"WM_COMMANDHELP";
        g_mapMsgStr[WM_COMPACTING                      ] = L"WM_COMPACTING";
        g_mapMsgStr[WM_COMPAREITEM                     ] = L"WM_COMPAREITEM";
        g_mapMsgStr[WM_CONTEXTMENU                     ] = L"WM_CONTEXTMENU";
        g_mapMsgStr[WM_WNT_CONVERTREQUESTEX            ] = L"WM_CONVERTREQUESTEX";
        g_mapMsgStr[WM_COPY                            ] = L"WM_COPY";
        g_mapMsgStr[WM_COPYDATA                        ] = L"WM_COPYDATA";
        g_mapMsgStr[WM_CREATE                          ] = L"WM_CREATE";
        g_mapMsgStr[WM_CTLCOLORBTN                     ] = L"WM_CTLCOLORBTN";
        g_mapMsgStr[WM_CTLCOLORDLG                     ] = L"WM_CTLCOLORDLG";
        g_mapMsgStr[WM_CTLCOLOREDIT                    ] = L"WM_CTLCOLOREDIT";
        g_mapMsgStr[WM_CTLCOLORLISTBOX                 ] = L"WM_CTLCOLORLISTBOX";
        g_mapMsgStr[WM_CTLCOLORMSGBOX                  ] = L"WM_CTLCOLORMSGBOX";
        g_mapMsgStr[WM_CTLCOLORSCROLLBAR               ] = L"WM_CTLCOLORSCROLLBAR";
        g_mapMsgStr[WM_CTLCOLORSTATIC                  ] = L"WM_CTLCOLORSTATIC";
        g_mapMsgStr[WM_CUT                             ] = L"WM_CUT";
        g_mapMsgStr[WM_DDE_ACK                         ] = L"WM_DDE_ACK";
        g_mapMsgStr[WM_DDE_ADVISE                      ] = L"WM_DDE_ADVISE";
        g_mapMsgStr[WM_DDE_DATA                        ] = L"WM_DDE_DATA";
        g_mapMsgStr[WM_DDE_EXECUTE                     ] = L"WM_DDE_EXECUTE";
        g_mapMsgStr[WM_DDE_INITIATE                    ] = L"WM_DDE_INITIATE";
        g_mapMsgStr[WM_DDE_POKE                        ] = L"WM_DDE_POKE";
        g_mapMsgStr[WM_DDE_REQUEST                     ] = L"WM_DDE_REQUEST";
        g_mapMsgStr[WM_DDE_TERMINATE                   ] = L"WM_DDE_TERMINATE";
        g_mapMsgStr[WM_DDE_UNADVISE                    ] = L"WM_DDE_UNADVISE";
        g_mapMsgStr[WM_DEADCHAR                        ] = L"WM_DEADCHAR";
        g_mapMsgStr[WM_DELETEITEM                      ] = L"WM_DELETEITEM";
        g_mapMsgStr[WM_DESTROY                         ] = L"WM_DESTROY";
        g_mapMsgStr[WM_DESTROYCLIPBOARD                ] = L"WM_DESTROYCLIPBOARD";
        g_mapMsgStr[WM_DEVICECHANGE                    ] = L"WM_DEVICECHANGE";
        g_mapMsgStr[WM_DEVMODECHANGE                   ] = L"WM_DEVMODECHANGE";
        g_mapMsgStr[WM_DISABLEMODAL                    ] = L"WM_DISABLEMODAL";
        g_mapMsgStr[WM_DISPLAYCHANGE                   ] = L"WM_DISPLAYCHANGE";
        g_mapMsgStr[WM_DRAWCLIPBOARD                   ] = L"WM_DRAWCLIPBOARD";
        g_mapMsgStr[WM_DRAWITEM                        ] = L"WM_DRAWITEM";
        g_mapMsgStr[WM_DROPFILES                       ] = L"WM_DROPFILES";
        g_mapMsgStr[WM_DWMCOLORIZATIONCOLORCHANGED     ] = L"WM_DWMCOLORIZATIONCOLORCHANGED";
        g_mapMsgStr[WM_DWMCOMPOSITIONCHANGED           ] = L"WM_DWMCOMPOSITIONCHANGED";
        g_mapMsgStr[WM_DWMNCRENDERINGCHANGED           ] = L"WM_DWMNCRENDERINGCHANGED";
        g_mapMsgStr[WM_DWMSENDICONICLIVEPREVIEWBITMAP  ] = L"WM_DWMSENDICONICLIVEPREVIEWBITMAP";
        g_mapMsgStr[WM_DWMSENDICONICTHUMBNAIL          ] = L"WM_DWMSENDICONICTHUMBNAIL";
        g_mapMsgStr[WM_DWMWINDOWMAXIMIZEDCHANGE        ] = L"WM_DWMWINDOWMAXIMIZEDCHANGE";
        g_mapMsgStr[WM_ENABLE                          ] = L"WM_ENABLE";
        g_mapMsgStr[WM_ENDSESSION                      ] = L"WM_ENDSESSION";
        g_mapMsgStr[WM_ENTERIDLE                       ] = L"WM_ENTERIDLE";
        g_mapMsgStr[WM_ENTERMENULOOP                   ] = L"WM_ENTERMENULOOP";
        g_mapMsgStr[WM_ENTERSIZEMOVE                   ] = L"WM_ENTERSIZEMOVE";
        g_mapMsgStr[WM_ERASEBKGND                      ] = L"WM_ERASEBKGND";
        g_mapMsgStr[WM_EXITHELPMODE                    ] = L"WM_EXITHELPMODE";
        g_mapMsgStr[WM_EXITMENULOOP                    ] = L"WM_EXITMENULOOP";
        g_mapMsgStr[WM_EXITSIZEMOVE                    ] = L"WM_EXITSIZEMOVE";
        g_mapMsgStr[WM_FLOATSTATUS                     ] = L"WM_FLOATSTATUS";
        g_mapMsgStr[WM_FONTCHANGE                      ] = L"WM_FONTCHANGE";
        g_mapMsgStr[WM_GESTURE                         ] = L"WM_GESTURE";
        g_mapMsgStr[WM_GESTURENOTIFY                   ] = L"WM_GESTURENOTIFY";
        g_mapMsgStr[WM_GETDLGCODE                      ] = L"WM_GETDLGCODE";
        g_mapMsgStr[WM_GETFONT                         ] = L"WM_GETFONT";
        g_mapMsgStr[WM_GETHOTKEY                       ] = L"WM_GETHOTKEY";
        g_mapMsgStr[WM_GETICON                         ] = L"WM_GETICON";
        g_mapMsgStr[WM_GETMINMAXINFO                   ] = L"WM_GETMINMAXINFO";
        g_mapMsgStr[WM_GETOBJECT                       ] = L"WM_GETOBJECT";
        g_mapMsgStr[WM_GETTEXT                         ] = L"WM_GETTEXT";
        g_mapMsgStr[WM_GETTEXTLENGTH                   ] = L"WM_GETTEXTLENGTH";
        g_mapMsgStr[WM_GETTITLEBARINFOEX               ] = L"WM_GETTITLEBARINFOEX";
        g_mapMsgStr[WM_HELP                            ] = L"WM_HELP";
        g_mapMsgStr[WM_HELPHITTEST                     ] = L"WM_HELPHITTEST";
        g_mapMsgStr[WM_HOTKEY                          ] = L"WM_HOTKEY";
        g_mapMsgStr[WM_HSCROLL                         ] = L"WM_HSCROLL";
        g_mapMsgStr[WM_HSCROLLCLIPBOARD                ] = L"WM_HSCROLLCLIPBOARD";
        g_mapMsgStr[WM_ICONERASEBKGND                  ] = L"WM_ICONERASEBKGND";
        g_mapMsgStr[WM_IDLEUPDATECMDUI                 ] = L"WM_IDLEUPDATECMDUI";
        g_mapMsgStr[WM_IME_CHAR                        ] = L"WM_IME_CHAR";
        g_mapMsgStr[WM_IME_COMPOSITION                 ] = L"WM_IME_COMPOSITION";
        g_mapMsgStr[WM_IME_COMPOSITIONFULL             ] = L"WM_IME_COMPOSITIONFULL";
        g_mapMsgStr[WM_IME_CONTROL                     ] = L"WM_IME_CONTROL";
        g_mapMsgStr[WM_IME_ENDCOMPOSITION              ] = L"WM_IME_ENDCOMPOSITION";
        g_mapMsgStr[WM_IME_KEYDOWN                     ] = L"WM_IME_KEYDOWN";
        g_mapMsgStr[WM_IME_KEYUP                       ] = L"WM_IME_KEYUP";
        g_mapMsgStr[WM_IME_NOTIFY                      ] = L"WM_IME_NOTIFY";
        g_mapMsgStr[WM_IME_REQUEST                     ] = L"WM_IME_REQUEST";
        g_mapMsgStr[WM_IME_SELECT                      ] = L"WM_IME_SELECT";
        g_mapMsgStr[WM_IME_SETCONTEXT                  ] = L"WM_IME_SETCONTEXT";
        g_mapMsgStr[WM_IME_STARTCOMPOSITION            ] = L"WM_IME_STARTCOMPOSITION";
        g_mapMsgStr[WM_INITDIALOG                      ] = L"WM_INITDIALOG";
        g_mapMsgStr[WM_INITIALUPDATE                   ] = L"WM_INITIALUPDATE";
        g_mapMsgStr[WM_INITMENU                        ] = L"WM_INITMENU";
        g_mapMsgStr[WM_INITMENUPOPUP                   ] = L"WM_INITMENUPOPUP";
        g_mapMsgStr[WM_INPUT                           ] = L"WM_INPUT";
        g_mapMsgStr[WM_INPUT_DEVICE_CHANGE             ] = L"WM_INPUT_DEVICE_CHANGE";
        g_mapMsgStr[WM_INPUTLANGCHANGE                 ] = L"WM_INPUTLANGCHANGE";
        g_mapMsgStr[WM_INPUTLANGCHANGEREQUEST          ] = L"WM_INPUTLANGCHANGEREQUEST";
        g_mapMsgStr[WM_KEYDOWN                         ] = L"WM_KEYDOWN";
        g_mapMsgStr[WM_KEYUP                           ] = L"WM_KEYUP";
        g_mapMsgStr[WM_KICKIDLE                        ] = L"WM_KICKIDLE";
        g_mapMsgStr[WM_KILLFOCUS                       ] = L"WM_KILLFOCUS";
        g_mapMsgStr[WM_LBUTTONDBLCLK                   ] = L"WM_LBUTTONDBLCLK";
        g_mapMsgStr[WM_LBUTTONDOWN                     ] = L"WM_LBUTTONDOWN";
        g_mapMsgStr[WM_LBUTTONUP                       ] = L"WM_LBUTTONUP";
        g_mapMsgStr[WM_MBUTTONDBLCLK                   ] = L"WM_MBUTTONDBLCLK";
        g_mapMsgStr[WM_MBUTTONDOWN                     ] = L"WM_MBUTTONDOWN";
        g_mapMsgStr[WM_MBUTTONUP                       ] = L"WM_MBUTTONUP";
        g_mapMsgStr[WM_MDIACTIVATE                     ] = L"WM_MDIACTIVATE";
        g_mapMsgStr[WM_MDICASCADE                      ] = L"WM_MDICASCADE";
        g_mapMsgStr[WM_MDICREATE                       ] = L"WM_MDICREATE";
        g_mapMsgStr[WM_MDIDESTROY                      ] = L"WM_MDIDESTROY";
        g_mapMsgStr[WM_MDIGETACTIVE                    ] = L"WM_MDIGETACTIVE";
        g_mapMsgStr[WM_MDIICONARRANGE                  ] = L"WM_MDIICONARRANGE";
        g_mapMsgStr[WM_MDIMAXIMIZE                     ] = L"WM_MDIMAXIMIZE";
        g_mapMsgStr[WM_MDINEXT                         ] = L"WM_MDINEXT";
        g_mapMsgStr[WM_MDIREFRESHMENU                  ] = L"WM_MDIREFRESHMENU";
        g_mapMsgStr[WM_MDIRESTORE                      ] = L"WM_MDIRESTORE";
        g_mapMsgStr[WM_MDISETMENU                      ] = L"WM_MDISETMENU";
        g_mapMsgStr[WM_MDITILE                         ] = L"WM_MDITILE";
        g_mapMsgStr[WM_MEASUREITEM                     ] = L"WM_MEASUREITEM";
        g_mapMsgStr[WM_MENUCHAR                        ] = L"WM_MENUCHAR";
        g_mapMsgStr[WM_MENUCOMMAND                     ] = L"WM_MENUCOMMAND";
        g_mapMsgStr[WM_MENUDRAG                        ] = L"WM_MENUDRAG";
        g_mapMsgStr[WM_MENUGETOBJECT                   ] = L"WM_MENUGETOBJECT";
        g_mapMsgStr[WM_MENURBUTTONUP                   ] = L"WM_MENURBUTTONUP";
        g_mapMsgStr[WM_MENUSELECT                      ] = L"WM_MENUSELECT";
        g_mapMsgStr[WM_MOUSEACTIVATE                   ] = L"WM_MOUSEACTIVATE";
        g_mapMsgStr[WM_MOUSEHOVER                      ] = L"WM_MOUSEHOVER";
        g_mapMsgStr[WM_MOUSEHWHEEL                     ] = L"WM_MOUSEHWHEEL";
        g_mapMsgStr[WM_MOUSELEAVE                      ] = L"WM_MOUSELEAVE";
        g_mapMsgStr[WM_MOUSEMOVE                       ] = L"WM_MOUSEMOVE";
        g_mapMsgStr[WM_MOUSEWHEEL                      ] = L"WM_MOUSEWHEEL";
        g_mapMsgStr[WM_MOVE                            ] = L"WM_MOVE";
        g_mapMsgStr[WM_MOVING                          ] = L"WM_MOVING";
        g_mapMsgStr[WM_NCACTIVATE                      ] = L"WM_NCACTIVATE";
        g_mapMsgStr[WM_NCCALCSIZE                      ] = L"WM_NCCALCSIZE";
        g_mapMsgStr[WM_NCCREATE                        ] = L"WM_NCCREATE";
        g_mapMsgStr[WM_NCDESTROY                       ] = L"WM_NCDESTROY";
        g_mapMsgStr[WM_NCHITTEST                       ] = L"WM_NCHITTEST";
        g_mapMsgStr[WM_NCLBUTTONDBLCLK                 ] = L"WM_NCLBUTTONDBLCLK";
        g_mapMsgStr[WM_NCLBUTTONDOWN                   ] = L"WM_NCLBUTTONDOWN";
        g_mapMsgStr[WM_NCLBUTTONUP                     ] = L"WM_NCLBUTTONUP";
        g_mapMsgStr[WM_NCMBUTTONDBLCLK                 ] = L"WM_NCMBUTTONDBLCLK";
        g_mapMsgStr[WM_NCMBUTTONDOWN                   ] = L"WM_NCMBUTTONDOWN";
        g_mapMsgStr[WM_NCMBUTTONUP                     ] = L"WM_NCMBUTTONUP";
        g_mapMsgStr[WM_NCMOUSEHOVER                    ] = L"WM_NCMOUSEHOVER";
        g_mapMsgStr[WM_NCMOUSELEAVE                    ] = L"WM_NCMOUSELEAVE";
        g_mapMsgStr[WM_NCMOUSEMOVE                     ] = L"WM_NCMOUSEMOVE";
        g_mapMsgStr[WM_NCPAINT                         ] = L"WM_NCPAINT";
        g_mapMsgStr[WM_NCRBUTTONDBLCLK                 ] = L"WM_NCRBUTTONDBLCLK";
        g_mapMsgStr[WM_NCRBUTTONDOWN                   ] = L"WM_NCRBUTTONDOWN";
        g_mapMsgStr[WM_NCRBUTTONUP                     ] = L"WM_NCRBUTTONUP";
        g_mapMsgStr[WM_NCXBUTTONDBLCLK                 ] = L"WM_NCXBUTTONDBLCLK";
        g_mapMsgStr[WM_NCXBUTTONDOWN                   ] = L"WM_NCXBUTTONDOWN";
        g_mapMsgStr[WM_NCXBUTTONUP                     ] = L"WM_NCXBUTTONUP";
        g_mapMsgStr[WM_NEXTDLGCTL                      ] = L"WM_NEXTDLGCTL";
        g_mapMsgStr[WM_NEXTMENU                        ] = L"WM_NEXTMENU";
        g_mapMsgStr[WM_NOTIFY                          ] = L"WM_NOTIFY";
        g_mapMsgStr[WM_NOTIFYFORMAT                    ] = L"WM_NOTIFYFORMAT";
        g_mapMsgStr[WM_NULL                            ] = L"WM_NULL";
        g_mapMsgStr[WM_PAINT                           ] = L"WM_PAINT";
        g_mapMsgStr[WM_PAINTCLIPBOARD                  ] = L"WM_PAINTCLIPBOARD";
        g_mapMsgStr[WM_PAINTICON                       ] = L"WM_PAINTICON";
        g_mapMsgStr[WM_PALETTECHANGED                  ] = L"WM_PALETTECHANGED";
        g_mapMsgStr[WM_PALETTEISCHANGING               ] = L"WM_PALETTEISCHANGING";
        g_mapMsgStr[WM_PARENTNOTIFY                    ] = L"WM_PARENTNOTIFY";
        g_mapMsgStr[WM_PASTE                           ] = L"WM_PASTE";
        g_mapMsgStr[WM_POPMESSAGESTRING                ] = L"WM_POPMESSAGESTRING";
        g_mapMsgStr[WM_POWER                           ] = L"WM_POWER";
        g_mapMsgStr[WM_POWERBROADCAST                  ] = L"WM_POWERBROADCAST";
        g_mapMsgStr[WM_PRINT                           ] = L"WM_PRINT";
        g_mapMsgStr[WM_PRINTCLIENT                     ] = L"WM_PRINTCLIENT";
        g_mapMsgStr[WM_PSD_ENVSTAMPRECT                ] = L"WM_PSD_ENVSTAMPRECT";
        g_mapMsgStr[WM_PSD_FULLPAGERECT                ] = L"WM_PSD_FULLPAGERECT";
        g_mapMsgStr[WM_PSD_GREEKTEXTRECT               ] = L"WM_PSD_GREEKTEXTRECT";
        g_mapMsgStr[WM_PSD_MARGINRECT                  ] = L"WM_PSD_MARGINRECT";
        g_mapMsgStr[WM_PSD_MINMARGINRECT               ] = L"WM_PSD_MINMARGINRECT";
        g_mapMsgStr[WM_PSD_PAGESETUPDLG                ] = L"WM_PSD_PAGESETUPDLG";
        g_mapMsgStr[WM_PSD_YAFULLPAGERECT              ] = L"WM_PSD_YAFULLPAGERECT";
        g_mapMsgStr[WM_QUERYAFXWNDPROC                 ] = L"WM_QUERYAFXWNDPROC";
        g_mapMsgStr[WM_QUERYCENTERWND                  ] = L"WM_QUERYCENTERWND";
        g_mapMsgStr[WM_QUERYDRAGICON                   ] = L"WM_QUERYDRAGICON";
        g_mapMsgStr[WM_QUERYENDSESSION                 ] = L"WM_QUERYENDSESSION";
        g_mapMsgStr[WM_QUERYNEWPALETTE                 ] = L"WM_QUERYNEWPALETTE";
        g_mapMsgStr[WM_QUERYOPEN                       ] = L"WM_QUERYOPEN";
        g_mapMsgStr[WM_QUERYUISTATE                    ] = L"WM_QUERYUISTATE";
        g_mapMsgStr[WM_QUEUE_SENTINEL                  ] = L"WM_QUEUE_SENTINEL";
        g_mapMsgStr[WM_QUEUESYNC                       ] = L"WM_QUEUESYNC";
        g_mapMsgStr[WM_QUIT                            ] = L"WM_QUIT";
        g_mapMsgStr[WM_RBUTTONDBLCLK                   ] = L"WM_RBUTTONDBLCLK";
        g_mapMsgStr[WM_RBUTTONDOWN                     ] = L"WM_RBUTTONDOWN";
        g_mapMsgStr[WM_RBUTTONUP                       ] = L"WM_RBUTTONUP";
        g_mapMsgStr[WM_RECALCPARENT                    ] = L"WM_RECALCPARENT";
        g_mapMsgStr[WM_RENDERALLFORMATS                ] = L"WM_RENDERALLFORMATS";
        g_mapMsgStr[WM_RENDERFORMAT                    ] = L"WM_RENDERFORMAT";
        g_mapMsgStr[WM_SETCURSOR                       ] = L"WM_SETCURSOR";
        g_mapMsgStr[WM_SETFOCUS                        ] = L"WM_SETFOCUS";
        g_mapMsgStr[WM_SETFONT                         ] = L"WM_SETFONT";
        g_mapMsgStr[WM_SETHOTKEY                       ] = L"WM_SETHOTKEY";
        g_mapMsgStr[WM_SETICON                         ] = L"WM_SETICON";
        g_mapMsgStr[WM_SETMESSAGESTRING                ] = L"WM_SETMESSAGESTRING";
        g_mapMsgStr[WM_SETREDRAW                       ] = L"WM_SETREDRAW";
        g_mapMsgStr[WM_SETTEXT                         ] = L"WM_SETTEXT";
        g_mapMsgStr[WM_SETTINGCHANGE                   ] = L"WM_SETTINGCHANGE";
        g_mapMsgStr[WM_SHOWWINDOW                      ] = L"WM_SHOWWINDOW";
        g_mapMsgStr[WM_SIZE                            ] = L"WM_SIZE";
        g_mapMsgStr[WM_SIZECHILD                       ] = L"WM_SIZECHILD";
        g_mapMsgStr[WM_SIZECLIPBOARD                   ] = L"WM_SIZECLIPBOARD";
        g_mapMsgStr[WM_SIZING                          ] = L"WM_SIZING";
        g_mapMsgStr[WM_SIZEPARENT                      ] = L"WM_SIZEPARENT";
        g_mapMsgStr[WM_SOCKET_NOTIFY                   ] = L"WM_SOCKET_NOTIFY";
        g_mapMsgStr[WM_SOCKET_DEAD                     ] = L"WM_SOCKET_DEAD";
        g_mapMsgStr[WM_SPOOLERSTATUS                   ] = L"WM_SPOOLERSTATUS";
        g_mapMsgStr[WM_STYLECHANGED                    ] = L"WM_STYLECHANGED";
        g_mapMsgStr[WM_STYLECHANGING                   ] = L"WM_STYLECHANGING";
        g_mapMsgStr[WM_SYNCPAINT                       ] = L"WM_SYNCPAINT";
        g_mapMsgStr[WM_SYSCHAR                         ] = L"WM_SYSCHAR";
        g_mapMsgStr[WM_SYSCOLORCHANGE                  ] = L"WM_SYSCOLORCHANGE";
        g_mapMsgStr[WM_SYSCOMMAND                      ] = L"WM_SYSCOMMAND";
        g_mapMsgStr[WM_SYSDEADCHAR                     ] = L"WM_SYSDEADCHAR";
        g_mapMsgStr[WM_SYSKEYDOWN                      ] = L"WM_SYSKEYDOWN";
        g_mapMsgStr[WM_SYSKEYUP                        ] = L"WM_SYSKEYUP";
        g_mapMsgStr[WM_TCARD                           ] = L"WM_TCARD";
        g_mapMsgStr[WM_THEMECHANGED                    ] = L"WM_THEMECHANGED";
        g_mapMsgStr[WM_TIMECHANGE                      ] = L"WM_TIMECHANGE";
        g_mapMsgStr[WM_TIMER                           ] = L"WM_TIMER";
        g_mapMsgStr[WM_TOUCH                           ] = L"WM_TOUCH";
        g_mapMsgStr[WM_UNDO                            ] = L"WM_UNDO";
        g_mapMsgStr[WM_UNICHAR                         ] = L"WM_UNICHAR";
        g_mapMsgStr[WM_UNINITMENUPOPUP                 ] = L"WM_UNINITMENUPOPUP";
        g_mapMsgStr[WM_UPDATEUISTATE                   ] = L"WM_UPDATEUISTATE";
        g_mapMsgStr[WM_USERCHANGED                     ] = L"WM_USERCHANGED";
        g_mapMsgStr[WM_VKEYTOITEM                      ] = L"WM_VKEYTOITEM";
        g_mapMsgStr[WM_VSCROLL                         ] = L"WM_VSCROLL";
        g_mapMsgStr[WM_VSCROLLCLIPBOARD                ] = L"WM_VSCROLLCLIPBOARD";
        g_mapMsgStr[WM_WINDOWPOSCHANGED                ] = L"WM_WINDOWPOSCHANGED";
        g_mapMsgStr[WM_WINDOWPOSCHANGING               ] = L"WM_WINDOWPOSCHANGING";
        g_mapMsgStr[WM_WTSSESSION_CHANGE               ] = L"WM_WTSSESSION_CHANGE";
        g_mapMsgStr[WM_XBUTTONDBLCLK                   ] = L"WM_XBUTTONDBLCLK";
        g_mapMsgStr[WM_XBUTTONDOWN                     ] = L"WM_XBUTTONDOWN";
        g_mapMsgStr[WM_XBUTTONUP                       ] = L"WM_XBUTTONUP";
    }

    if (g_mapMsgStr.find(uMsg) != g_mapMsgStr.end())
    {
        return g_mapMsgStr[uMsg];
    }

    return L"";
}

#ifdef __cplusplus
}
#endif
