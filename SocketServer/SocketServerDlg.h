
// SocketServerDlg.h: 標頭檔
//

#pragma once
#define WM_SHOW_DATA WM_USER + 100


// CSocketServerDlg 對話方塊
class CSocketServerDlg : public CDialogEx
{
// 建構
public:
	CSocketServerDlg(CWnd* pParent = nullptr);	// 標準建構函式

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SOCKETSERVER_DIALOG };
#endif

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);// DDX/DDV 支援


// 程式碼實作
public:
	static SOCKET ClientSocket[1024];
	SOCKET ServerSocket;
protected:
	HICON m_hIcon;
	CEdit m_edit1;
	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	BOOL PreTranslateMessage(MSG* pMsg);
	LRESULT OnUpdateEdit1(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButton2();
};
