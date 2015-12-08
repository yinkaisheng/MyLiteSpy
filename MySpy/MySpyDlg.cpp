
// CbtHookDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MySpy.h"
#include "MySpyDlg.h"
#include "afxdialogex.h"
#include "../myhook/myhook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // 对话框数据
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    // 实现
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMySpyDlg 对话框



CMySpyDlg::CMySpyDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CMySpyDlg::IDD, pParent)
    , m_bCBTHooked(false)
    , m_bCallWndProcHooked(false)
    , m_bCallWndProcRetHooked(false)
    , m_bGetMessageHooked(false)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMySpyDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_HWND, m_editWnd);
    DDX_Control(pDX, IDC_CHECK_TOPMOST, m_checkTopMost);
    DDX_Control(pDX, IDC_EDIT_OUTPUT, m_editOutput);
}

BEGIN_MESSAGE_MAP(CMySpyDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_NotifyActivate, &CMySpyDlg::OnNotifyActivate)
    ON_MESSAGE(WM_NotifyActivate, &CMySpyDlg::OnNotifySetFocus)
    ON_MESSAGE(WM_NotifyGetMessage, &CMySpyDlg::OnNotifyGetMessage)
    ON_MESSAGE(WM_NotifyCallWndProc, &CMySpyDlg::OnNotifyCallWndProc)
    ON_MESSAGE(WM_NotifyCallWndProcRet, &CMySpyDlg::OnNotifyCallWndProcRet)
    ON_BN_CLICKED(IDC_BUTTON_INSTALL_CBT, &CMySpyDlg::OnBnClickedButtonInstallCBT)
    ON_BN_CLICKED(IDC_BUTTON_UNINSTALL_CBT, &CMySpyDlg::OnBnClickedButtonUninstallCBT)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BUTTON_INSTALL_SPY, &CMySpyDlg::OnBnClickedButtonInstallSpy)
    ON_BN_CLICKED(IDC_BUTTON_UNINSTALL_SPY, &CMySpyDlg::OnBnClickedButtonUninstallSpy)
    ON_BN_CLICKED(IDC_CHECK_TOPMOST, &CMySpyDlg::OnBnClickedCheckTopmost)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CMySpyDlg 消息处理程序

BOOL CMySpyDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 将“关于...”菜单项添加到系统菜单中。

    // IDM_ABOUTBOX 必须在系统命令范围内。
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    // TODO: 在此添加额外的初始化代码

    if (8 == sizeof(void*))
    {
        SetWindowText(TEXT("MySpy_x64"));
    }
    else
    {
        SetWindowText(TEXT("MySpy_Win32"));
    }

    m_editOutput.SetLimitText(0xFFFFFFFF);

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMySpyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMySpyDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMySpyDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CMySpyDlg::OnBnClickedButtonInstallCBT()
{
    // TODO: 在此添加控件通知处理程序代码
    if (!m_bCBTHooked)
    {
        m_bCBTHooked = InstallCBTHook(m_hWnd);
        if (m_bCBTHooked)
        {
            OutputString(TEXT("CBTHook installed\n"));
        }
    }
}

void CMySpyDlg::OnBnClickedButtonUninstallCBT()
{
    // TODO: 在此添加控件通知处理程序代码
    if (m_bCBTHooked)
    {
        UninstallCBTHook();
        m_bCBTHooked = false;
        OutputString(TEXT("CBTHook uninstalled\n"));
    }
}

LRESULT CMySpyDlg::OnNotifyActivate(WPARAM wParam, LPARAM lParam)
{
    HWND hActivateWnd = (HWND)wParam;
    TCHAR szClassName[MAX_PATH] = {0};
    GetClassName(hActivateWnd, szClassName, MAX_PATH);
    OutputString(TEXT("Activate hWnd: 0x%X, ClassName: %s\n"), hActivateWnd, szClassName);

    return 0;
}

LRESULT CMySpyDlg::OnNotifySetFocus(WPARAM wParam, LPARAM lParam)
{
    HWND hFocusWnd = (HWND)wParam;
    TCHAR szClassName[MAX_PATH] = {0};
    GetClassName(hFocusWnd, szClassName, MAX_PATH);
    OutputString(TEXT("Focus hWnd: 0x%X, ClassName: %s\n"), hFocusWnd, szClassName);

    return 0;
}

LRESULT CMySpyDlg::OnNotifyCallWndProc(WPARAM wParam, LPARAM lParam)
{
    HWND hWnd = (HWND)wParam;
    UINT nMsg = (UINT)lParam;
    TCHAR szClassName[MAX_PATH] = {0};
    GetClassName(hWnd, szClassName, MAX_PATH);
    OutputString(TEXT("S hWnd: 0x%X, msg: %s(%d), ClassName: %s\r\n"), hWnd, GetMsgStringW(nMsg), nMsg, szClassName);

    return 0;
}

LRESULT CMySpyDlg::OnNotifyCallWndProcRet(WPARAM wParam, LPARAM lParam)
{
    HWND hWnd = (HWND)wParam;
    UINT nMsg = (UINT)lParam;
    TCHAR szClassName[MAX_PATH] = {0};
    GetClassName(hWnd, szClassName, MAX_PATH);
    OutputString(TEXT("R hWnd: 0x%X, msg: %s(%d), ClassName: %s\r\n"), hWnd, GetMsgStringW(nMsg), nMsg, szClassName);

    if (WM_NCDESTROY == nMsg)
    {
        OnBnClickedButtonUninstallSpy();
    }

    return 0;
}

LRESULT CMySpyDlg::OnNotifyGetMessage(WPARAM wParam, LPARAM lParam)
{
    HWND hWnd = (HWND)wParam;
    UINT nMsg = (UINT)lParam;
    TCHAR szClassName[MAX_PATH] = {0};
    GetClassName(hWnd, szClassName, MAX_PATH);
    OutputString(TEXT("P hWnd: 0x%X, msg: %s(%d), ClassName: %s\r\n"), hWnd, GetMsgStringW(nMsg), nMsg, szClassName);

    return 0;
}

void CMySpyDlg::OutputString(const WCHAR* pszFormat, ...)
{
    WCHAR szOutput[MAX_PATH];
    va_list args;
    va_start(args, pszFormat);
    _vsnwprintf_s(szOutput, _TRUNCATE, pszFormat, args);
    va_end(args);
    OutputDebugStringW(szOutput);

    m_editOutput.SetSel(-1, -1);
    m_editOutput.ReplaceSel(szOutput);
}


void CMySpyDlg::OnDestroy()
{
    CDialogEx::OnDestroy();
    OnBnClickedButtonUninstallCBT();
    OnBnClickedButtonUninstallSpy();
    // TODO: 在此处添加消息处理程序代码
}

void CMySpyDlg::OnBnClickedButtonInstallSpy()
{
    // TODO: 在此添加控件通知处理程序代码
    if (m_bCallWndProcHooked || m_bCallWndProcRetHooked || m_bGetMessageHooked)
    {
        OnBnClickedButtonUninstallSpy();
    }

    UpdateData(TRUE);
    TCHAR szValue[24] = {0};
    ::GetWindowText(::GetDlgItem(m_hWnd, IDC_EDIT_HWND), szValue, 24);
    int nWnd = 0;
    swscanf_s(szValue, TEXT("%x"), &nWnd);
    HWND hWnd = (HWND)nWnd;

    if (!hWnd || !::IsWindow(hWnd))
    {
        MessageBox(TEXT("Please input a valid window handle."), TEXT("No!!!"), MB_OK|MB_ICONWARNING);
        m_editWnd.SetFocus();
        return;
    }

    if (GetCurrentThreadId() == GetWindowThreadProcessId(hWnd, NULL))
    {
        MessageBox(TEXT("Capturing MySpy's window is not allowed.\nPlease input another window."), TEXT("No!!!"), MB_OK|MB_ICONWARNING);
        m_editWnd.SetFocus();
        m_editWnd.SetSel(0, -1);
        return;
    }

    m_bCallWndProcHooked = InstallCallWndProcHook(m_hWnd, hWnd);
    if (m_bCallWndProcHooked)
    {
        OutputString(TEXT("Hook CallWndProc succeed\n"));
    }
    else
    {
        OutputString(TEXT("Hook CallWndProc failed\n"));
    }

    m_bCallWndProcRetHooked = InstallCallWndProcRetHook(m_hWnd, hWnd);
    if (m_bCallWndProcRetHooked)
    {
        OutputString(TEXT("Hook CallWndProcRet succeed\n"));
    }
    else
    {
        OutputString(TEXT("Hook CallWndProcRet failed\n"));
    }

    m_bGetMessageHooked = InstallGetMessageHook(m_hWnd, hWnd);
    if (m_bGetMessageHooked)
    {
        OutputString(TEXT("Hook GetMessage succeed\n"));
    }
    else
    {
        OutputString(TEXT("Hook GetMessage failed\n"));
    }
}


void CMySpyDlg::OnBnClickedButtonUninstallSpy()
{
    if (m_bCallWndProcHooked)
    {
        UninstallCallWndProcHook();
        m_bCallWndProcHooked = false;
        OutputString(TEXT("Unhook CallWndProc\n"));
    }

    if (m_bCallWndProcRetHooked)
    {
        UninstallCallWndProcRetHook();
        m_bCallWndProcRetHooked = false;
        OutputString(TEXT("Unhook CallWndProcRet\n"));
    }

    if (m_bGetMessageHooked)
    {
        UninstallGetMessageHook();
        m_bGetMessageHooked = false;
        OutputString(TEXT("Unhook GetMessage\n"));
    }
}


void CMySpyDlg::OnBnClickedCheckTopmost()
{
    // TODO: 在此添加控件通知处理程序代码
    if (BST_CHECKED == m_checkTopMost.GetCheck())
    {
        ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
    }
    else
    {
        ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
    }
}


void CMySpyDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (m_editOutput.GetSafeHwnd())
    {
        m_editOutput.MoveWindow(12, 45, cx-24, cy-45-12);
    }
    // TODO: 在此处添加消息处理程序代码
}
