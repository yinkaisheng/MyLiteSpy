
// CbtHookDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CMySpyDlg 对话框
class CMySpyDlg : public CDialogEx
{
    // 构造
public:
    CMySpyDlg(CWnd* pParent = NULL);	// 标准构造函数

    // 对话框数据
    enum { IDD = IDD_MYSPY_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


    // 实现
protected:
    HICON m_hIcon;

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedButtonInstallCBT();
    afx_msg void OnBnClickedButtonUninstallCBT();
    afx_msg void OnBnClickedButtonInstallSpy();
    afx_msg void OnBnClickedButtonUninstallSpy();
    afx_msg void OnBnClickedCheckTopmost();
    afx_msg LRESULT OnNotifyActivate(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnNotifySetFocus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnNotifyCallWndProc(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnNotifyCallWndProcRet(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnNotifyGetMessage(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

    void OutputString(const WCHAR* pszFormat, ...);
    CString GetMessageString(UINT nMsg);

private:
    bool m_bCBTHooked;
    bool m_bCallWndProcHooked;
    bool m_bCallWndProcRetHooked;
    bool m_bGetMessageHooked;
    CEdit m_editWnd;
    CEdit m_editOutput;
    CButton m_checkTopMost;
};
