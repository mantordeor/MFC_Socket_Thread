// SocketServerDlg.cpp: 實作檔案
//

#include "pch.h"
#include "framework.h"
#include "SocketServer.h"
#include "SocketServerDlg.h"
#include "afxdialogex.h"
#pragma warning(disable:4996) 
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SOCKET CSocketServerDlg::ClientSocket[1024];
// 對 App About 使用 CAboutDlg 對話方塊

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
protected:
	DECLARE_MESSAGE_MAP()
};

struct CommunicateParams {
	CSocketServerDlg* dlg;
	int index;
};
struct AcceptThreadParams {
	SOCKET ServerSocket;
	HWND hWnd; // 主視窗控制代碼，用於 PostMessage
};
CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSocketServerDlg 對話方塊



CSocketServerDlg::CSocketServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SOCKETSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSocketServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edit1);
}

BEGIN_MESSAGE_MAP(CSocketServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CSocketServerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSocketServerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CSocketServerDlg::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT1, &CSocketServerDlg::OnEnChangeEdit1)
	ON_MESSAGE(WM_SHOW_DATA, &CSocketServerDlg::OnUpdateEdit1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSocketServerDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CSocketServerDlg 訊息處理常式

BOOL CSocketServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 將 [關於...] 功能表加入系統功能表。

	// IDM_ABOUTBOX 必須在系統命令範圍之中。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	// TODO: 在此加入額外的初始設定

	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CSocketServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CSocketServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CSocketServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSocketServerDlg::OnBnClickedOk()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	//CDialogEx::OnOK();
}

void CSocketServerDlg::OnBnClickedCancel()
{
	for (int i = 0; i < 1024; i++) {
		if (ClientSocket[i] != INVALID_SOCKET) {
			closesocket(ClientSocket[i]);
		}
	}
	closesocket(ServerSocket);
	WSACleanup();
	// TODO: 在此加入控制項告知處理常式程式碼
	CDialogEx::OnCancel();
}

// 修改 communicate 執行緒，將收到的 buff 轉為 Unicode 傳遞給主執行緒

void LogAction(const CString& action) {
	time_t now = time(0);
	tm localTime;
	localtime_s(&localTime, &now);
	char dateBuf[20];
	char timeBuf[20];
	strftime(dateBuf, sizeof dateBuf, "%Y-%m-%d", &localTime);
	strftime(timeBuf, sizeof timeBuf, "%Y-%m-%d %H:%M:%S", &localTime);
	CString currentDate = CString(dateBuf);
	CString filename;
	filename.Format(_T("Server_log_%s.txt"), currentDate);
	std::ofstream logFile(filename, std::ios::app);
	if (logFile.is_open()) {
		USES_CONVERSION;
		logFile << "[" << timeBuf << "] " << CT2A(action) << std::endl;
		logFile.close();
	}
	else {
		CString errorMsg;
		errorMsg.Format(_T("無法開啟日誌檔案 %s"), filename);
		OutputDebugString(errorMsg);
	}
}

DWORD WINAPI communicate(LPVOID lpParam) {
	CommunicateParams* params = (CommunicateParams*)lpParam;
	CSocketServerDlg* dlg = params->dlg;
	int idx = params->index;
	int r;
	char buff[1024];
	while (1) {
		r = recv(CSocketServerDlg::ClientSocket[idx], buff, sizeof buff - 1, 0);
		if (r > 0) {
			buff[r] = 0;
			int wlen = MultiByteToWideChar(CP_UTF8, 0, buff, -1, NULL, 0);
			CStringW wstr;
			LPWSTR wbuf = wstr.GetBuffer(wlen);
			MultiByteToWideChar(CP_UTF8, 0, buff, -1, wbuf, wlen);
			wstr.ReleaseBuffer();
			CStringW* msg = new CStringW;
			msg->Format(L"\r\n[Thread %d] %s", idx, wstr.GetString());
			dlg->PostMessage(WM_SHOW_DATA, (WPARAM)msg, 0);
		}
		else if (r == 0) {
			closesocket(CSocketServerDlg::ClientSocket[idx]);
			CSocketServerDlg::ClientSocket[idx] = INVALID_SOCKET;
			break;
		}
		else {
			closesocket(CSocketServerDlg::ClientSocket[idx]);
			CSocketServerDlg::ClientSocket[idx] = INVALID_SOCKET;
			break;
		}
	}
	delete params;
	return 0;
}
LRESULT CSocketServerDlg::OnUpdateEdit1(WPARAM wParam, LPARAM lParam)
{
    CStringW* receivedData = (CStringW*)wParam;
    CString currentText;
    m_edit1.GetWindowText(currentText);
    m_edit1.SetWindowText(currentText + *receivedData);
    int nLen = m_edit1.GetWindowTextLength();
    //m_edit1.SetSel(nLen, nLen);
    m_edit1.LineScroll(m_edit1.GetLineCount());
    delete receivedData;
    return 0;
}

DWORD WINAPI AcceptThread(LPVOID lpParam) {
	AcceptThreadParams* params = (AcceptThreadParams*)lpParam;
	SOCKET ServerSocket = params->ServerSocket;
	HWND hWnd = params->hWnd;
	SOCKADDR_IN ClientAddr = { 0 };
	int CADLEN = sizeof ClientAddr;
	for (int i = 0; i < 1024; i++) {
		CSocketServerDlg::ClientSocket[i] = accept(ServerSocket, (struct sockaddr*)&ClientAddr, &CADLEN);
        char ipStr[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &ClientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
        CString ipLog;
        ipLog.Format(_T("Client connected: %S"), ipStr);
        LogAction(ipLog);
		if(CSocketServerDlg::ClientSocket[i] == INVALID_SOCKET) {
			AfxMessageBox(L"Accept Failed!");
			LogAction(L"Accept Failed!");
			WSACleanup();
			delete params;
			break;
		}
		CommunicateParams* commParams = new CommunicateParams;
		commParams->dlg = (CSocketServerDlg*)CWnd::FromHandle(hWnd);
		commParams->index = i;
		HANDLE hThread = CreateThread(NULL, 0, communicate, commParams, 0, NULL);
		if (hThread == NULL) {
			AfxMessageBox(L"CreateThread Failed!");
			LogAction(L"CreateThread Failed!");
			WSACleanup();
			delete commParams;
			break;
		}
		else {
			CloseHandle(hThread);
		}
	}
	closesocket(ServerSocket);
	WSACleanup();
	return 0;

}

BOOL CSocketServerDlg::PreTranslateMessage(MSG* pMsg) {
	if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_CHAR || pMsg->message == WM_KEYUP) &&
		GetFocus() == GetDlgItem(IDC_EDIT1)) {
		return TRUE; 
	}
	if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONDBLCLK ||
		pMsg->message == WM_RBUTTONDOWN || pMsg->message == WM_RBUTTONDBLCLK ||
		pMsg->message == WM_MBUTTONDOWN || pMsg->message == WM_MBUTTONDBLCLK) {
		CPoint pt = pMsg->pt;
		ScreenToClient(&pt);
		CWnd* pWnd = ChildWindowFromPoint(pt);
		if (pWnd && pWnd->GetDlgCtrlID() == IDC_EDIT1) {
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

// 1. 在 OnBnClickedButton1 鎖定按鈕
void CSocketServerDlg::OnBnClickedButton1()
{
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE); // 鎖定按鈕

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (HIBYTE(wsaData.wVersion) != 2 || LOBYTE(wsaData.wVersion) != 2) {
		printf("Initialize Winsock Failed: %u\n", GetLastError());
		LogAction(L"Initialize Winsock Failed");
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE); // 初始化失敗時恢復按鈕
		return;
	}
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == -1) {
		AfxMessageBox(L"Winsocket Failed!");
		LogAction(L"Winsocket Failed!");
		WSACleanup();
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
		return;
	}
	SOCKADDR_IN ServerAddr;
	ServerAddr.sin_family = AF_INET;
	InetPton(AF_INET, _T("127.0.0.1"), &ServerAddr.sin_addr.S_un.S_addr);
	ServerAddr.sin_port = htons(7000);
	int r = bind(ServerSocket, (struct sockaddr*)&ServerAddr, sizeof ServerAddr);
	if (r == -1) {
		AfxMessageBox(L"Bind Failed!");
		LogAction(L"Bind Failed!");
		WSACleanup();
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
		return;
	}
	r = listen(ServerSocket, 10);
	if (r == -1) {
		AfxMessageBox(L"Listen Failed!");
		LogAction(L"Listen Failed!");
		WSACleanup();
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
		return;
	}
	AcceptThreadParams* params = new AcceptThreadParams;
	params->ServerSocket = ServerSocket;
	params->hWnd = this->m_hWnd;
	HANDLE hThread = CreateThread(NULL, 0, AcceptThread, params, 0, NULL);
	if(hThread == NULL) {
		AfxMessageBox(L"CreateThread Failed!");
		LogAction(L"CreateThread Failed!");
		WSACleanup();
		delete params;
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
		return;
	}
	SetDlgItemText(IDC_EDIT1, L"Start Listen");
	LogAction(L"Start Listen");
}




void CSocketServerDlg::OnEnChangeEdit1()
{
	// TODO:  如果這是 RICHEDIT 控制項，控制項將不會
	// 傳送此告知，除非您覆寫 CDialogEx::OnInitDialog()
	// 函式和呼叫 CRichEditCtrl().SetEventMask()
	// 讓具有 ENM_CHANGE 旗標 ORed 加入遮罩。

	// TODO:  在此加入控制項告知處理常式程式碼
}

void CSocketServerDlg::OnBnClickedButton2()
{
	for (int i = 0; i < 1024; i++) {
		if (ClientSocket[i] != INVALID_SOCKET) {
			closesocket(ClientSocket[i]);
			ClientSocket[i] = INVALID_SOCKET;
		}
	}
	if (ServerSocket != INVALID_SOCKET) {
		closesocket(ServerSocket);
		ServerSocket = INVALID_SOCKET;
	}
	WSACleanup();

	GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	SetDlgItemText(IDC_EDIT1, L"Bind Released");
	LogAction(L"Bind Released");
}
