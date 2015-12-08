#pragma once

#ifdef MYHOOK_EXPORTS
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

enum NotifyMsg
{
    WM_NotifyActivate = WM_APP+1,
    WM_NotifyFocus,
    WM_NotifyCallWndProc,
    WM_NotifyCallWndProcRet,
    WM_NotifyGetMessage,
};

#ifdef __cplusplus
extern "C"
{
#endif

DLL_EXPORT bool InstallCBTHook(HWND hNotifyWnd);
DLL_EXPORT bool UninstallCBTHook();

DLL_EXPORT bool InstallCallWndProcHook(HWND hNotifyWnd, HWND hCaptureWnd);
DLL_EXPORT bool UninstallCallWndProcHook();

DLL_EXPORT bool InstallCallWndProcRetHook(HWND hNotifyWnd, HWND hCaptureWnd);
DLL_EXPORT bool UninstallCallWndProcRetHook();

DLL_EXPORT bool InstallGetMessageHook(HWND hNotifyWnd, HWND hCaptureWnd);
DLL_EXPORT bool UninstallGetMessageHook();

DLL_EXPORT const char* GetMsgStringA(UINT uMsg);
DLL_EXPORT const WCHAR* GetMsgStringW(UINT uMsg);

#ifdef __cplusplus
}
#endif
