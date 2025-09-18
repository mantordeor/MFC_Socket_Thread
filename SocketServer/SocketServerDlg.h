
// SocketServerDlg.h: 標頭檔
//

#pragma once
#define WM_SHOW_DATA WM_USER + 100
struct ClientInfo {
	SOCKET socket;
	CString ipAddress;
	bool isConnected;
};

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
private:
	static SOCKET ClientSocket;
	SOCKET ServerSocket;
	UINT_PTR m_timerID;
	std::list<ClientInfo> m_clientList;
	HICON m_hIcon;
	CEdit m_edit1;
protected:
	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnAddIPAddress(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnCbnSelchangeCombo1();
	BOOL PreTranslateMessage(MSG* pMsg);
	LRESULT OnUpdateEdit1(WPARAM wParam, LPARAM lParam);
	CComboBox m_combo;
	afx_msg void OnBnClickedButton3();
	void DisconnectClient(CString ipAddress);
	void CheckClientConnections();
	void UpdateComboList();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEdit2();
};
