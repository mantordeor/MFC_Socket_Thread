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
SOCKET CSocketServerDlg::ClientSocket = -1;
// 對 App About 使用 CAboutDlg 對話方塊
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
public:
	afx_msg void OnBnClickedButton1();
};

struct CommunicateData {
	SOCKET socket;
	WSAEVENT event;
	char buffer[1024];
	CSocketServerDlg* dlg;
};
struct AcceptThreadParams {
	SOCKET ServerSocket;
	HWND hWnd;
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
	DDX_Control(pDX, IDC_COMBO1, m_combo);
}

BEGIN_MESSAGE_MAP(CSocketServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CSocketServerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSocketServerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CSocketServerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSocketServerDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CSocketServerDlg::OnBnClickedButton3)
	ON_MESSAGE(WM_SHOW_DATA, &CSocketServerDlg::OnUpdateEdit1)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CSocketServerDlg::OnCbnSelchangeCombo1)
	ON_MESSAGE(WM_USER + 101, &CSocketServerDlg::OnAddIPAddress)
	ON_EN_CHANGE(IDC_EDIT1, &CSocketServerDlg::OnEnChangeEdit1)
	ON_WM_TIMER()
	ON_EN_CHANGE(IDC_EDIT2, &CSocketServerDlg::OnEnChangeEdit2)
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
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	m_timerID = SetTimer(1, 5000, NULL);
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
	ClientSocket = -1;
	ServerSocket = -1;
	WSACleanup();
	// TODO: 在此加入控制項告知處理常式程式碼
	CDialogEx::OnCancel();
}

DWORD CALLBACK communicate(LPVOID lpParam) {
	CommunicateData* data = (CommunicateData*)lpParam;
	SOCKET clientSocket = data->socket;
	WSAEVENT event = data->event;
	CSocketServerDlg* dlg = data->dlg;
	int r = WSAEventSelect(clientSocket, event, FD_READ | FD_CLOSE);
	if (r == SOCKET_ERROR) {
		CString msg;
		msg.Format(_T("WSAEventSelect failed: %d"), WSAGetLastError());
		LogAction(msg);
		closesocket(clientSocket);
		CloseHandle(event);
		delete data;
		return 1;
	}
	while (1) {
		DWORD result = WSAWaitForMultipleEvents(1, &event, FALSE, WSA_INFINITE, FALSE);
		if (result == WSA_WAIT_FAILED) {
			CString msg;
			msg.Format(_T("WSAWaitForMultipleEvents failed: %d"), WSAGetLastError());
			LogAction(msg);
			break;
		}
		WSANETWORKEVENTS networkEvents;
		r = WSAEnumNetworkEvents(clientSocket, event, &networkEvents);
		if (r == SOCKET_ERROR) {
			CString msg;
			msg.Format(_T("WSAEnumNetworkEvents failed; %d"), WSAGetLastError());
			LogAction(msg);
			break;
		}
		if (networkEvents.lNetworkEvents & FD_READ) {
			if (networkEvents.iErrorCode[FD_READ_BIT] == 0) {
				int bytesReceived = recv(clientSocket, data->buffer, sizeof data->buffer, 0);
				if (bytesReceived > 0) {
					data->buffer[bytesReceived] = 0;
					const size_t len = strlen(data->buffer) + 1;
					wchar_t* wbuff = new wchar_t[len];
					mbstowcs(wbuff, data->buffer, len);
					CStringW* msg = new CStringW;
					msg->Format(L"\r\n%s", wbuff);
					dlg->PostMessage(WM_SHOW_DATA, (WPARAM)msg, 0);
				}
				else if (bytesReceived == 0) {
					CString msg;
					msg.Format(_T("Client closed connection"));
					LogAction(msg);
					break;
				}
				else {
					CString msg;
					msg.Format(_T("recv failed: %d"), WSAGetLastError());
					LogAction(msg);
					break;
				}
			}
			else {
				CString msg;
				msg.Format(_T("FD_READ error: %d"), networkEvents.iErrorCode[FD_READ_BIT]);
				LogAction(msg);
				break;
			}
		}
		if (networkEvents.lNetworkEvents & FD_CLOSE) {
			CString msg;
			msg.Format(_T("Client closed connection"));
			LogAction(msg);
			break;
		}
	}
	closesocket(clientSocket);
	CloseHandle(event);
	delete data;
	return 0;
}

LRESULT CSocketServerDlg::OnAddIPAddress(WPARAM wParam, LPARAM lParam) {
	wchar_t* ipAddress = (wchar_t*)lParam;
	CString strIPaddress(ipAddress);
	m_combo.AddString(strIPaddress);
	delete[] ipAddress;
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

void CSocketServerDlg::CheckClientConnections() {
	for (auto iter = m_clientList.begin(); iter != m_clientList.end(); iter++) {
		SOCKET clientSocket = iter->socket;
		if (clientSocket != INVALID_SOCKET) {
			char buffer[1];
			int result = recv(clientSocket, buffer, sizeof buffer, MSG_PEEK);
			if (result == SOCKET_ERROR) {
				int error = WSAGetLastError();
				if (error == WSAECONNRESET || error == WSAENOTCONN) {
					iter->isConnected = false;
					CString msg;
					msg.Format(_T("Client %s disconnected"), iter->ipAddress);
					closesocket(clientSocket);
					iter->socket = INVALID_SOCKET;
				}
			}
			else {
				iter->isConnected = true;
			}
			if (iter->socket == INVALID_SOCKET) {
				iter = m_clientList.erase(iter);
			}
			else {
				iter++;
			}
		}
	}
	UpdateComboList();
}

void CSocketServerDlg::UpdateComboList()
{
	m_combo.ResetContent();
	for (const auto& clientInfo : m_clientList)
	{
		CString itemText;
		itemText.Format(_T("%s (%s)"), clientInfo.ipAddress, clientInfo.isConnected ? _T("Connected") : _T("Disconnected"));
		m_combo.AddString(itemText);
	}
}

DWORD WINAPI AcceptThread(LPVOID lpParam) {
	AcceptThreadParams* params = (AcceptThreadParams*)lpParam;
	CComboBox* pComboBox = (CComboBox*)lpParam;
	SOCKET ServerSocket = params->ServerSocket;
	HWND hWnd = params->hWnd;
	SOCKADDR_IN ClientAddr = { 0 };
	int CADLEN = sizeof ClientAddr;
	while(1) {
		SOCKET clientSocket = accept(ServerSocket, (struct sockaddr*)&ClientAddr, &CADLEN);
		if (clientSocket == INVALID_SOCKET) {
			AfxMessageBox(L"socket failed in AcceptThread");
			break;
		}
		std::string ipStr = inet_ntoa(ClientAddr.sin_addr);
		int len = MultiByteToWideChar(CP_ACP, 0, ipStr.c_str(), -1, NULL, 0);
		wchar_t* wc = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, ipStr.c_str(), -1, wc, len);
		::PostMessage(hWnd, WM_USER + 101, 0, (LPARAM)wc);
		WSAEVENT event = WSACreateEvent();
		if (event == WSA_INVALID_EVENT) {
			CString msg;
			msg.Format(_T("WSACreateEvent failed: %d"), WSAGetLastError());
			LogAction(msg);
			closesocket(clientSocket);
			continue;
		}
		CommunicateData* data = new CommunicateData;
		data->socket = clientSocket;
		data->event = event;
		data->dlg = (CSocketServerDlg*)CWnd::FromHandle(hWnd);
		HANDLE hThread = CreateThread(NULL, 0, communicate, data, 0, NULL);
		if (hThread == NULL) {
			CString msg;
			msg.Format(_T("CreateThread failed: %d"), GetLastError());
			LogAction(msg);
			closesocket(clientSocket);
			CloseHandle(event);
			delete data;
			continue;
		}
		else CloseHandle(hThread);
	}
	WSACleanup();
	return 0;
}

void CSocketServerDlg::DisconnectClient(CString ipAddress) {
	for (auto iter = m_clientList.begin(); iter != m_clientList.end(); iter++) {
		if (iter->ipAddress == ipAddress) {
			SOCKET clientSocket = iter->socket;
			if (clientSocket != INVALID_SOCKET) {
				closesocket(clientSocket);
				CString msg;
				msg.Format(_T("disconnecting for %s"), ipAddress);
				AfxMessageBox(msg);
			}
			iter->socket = INVALID_SOCKET;
			break;
		}
	}
	m_clientList.remove_if([](const ClientInfo& client) { return client.socket == INVALID_SOCKET; });
	UpdateComboList();
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

void CSocketServerDlg::OnBnClickedButton1()
{
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (HIBYTE(wsaData.wVersion) != 2 || LOBYTE(wsaData.wVersion) != 2) {
		AfxMessageBox(L"Initialize Winsock Failed: %u\n", GetLastError());
		LogAction(L"Initialize Winsock Failed");
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE); 
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
	r = listen(ServerSocket, 64);
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
	closesocket(ClientSocket);
	closesocket(ServerSocket);
	WSACleanup();
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	SetDlgItemText(IDC_EDIT1, L"Bind Released");
	LogAction(L"Bind Released");
}

void CAboutDlg::OnBnClickedButton1()
{
	// TODO: 在此加入控制項告知處理常式程式碼
}

void CSocketServerDlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此加入控制項告知處理常式程式碼
}

void CSocketServerDlg::OnBnClickedButton3()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	int selectedIndex = m_combo.GetCurSel();
	if (selectedIndex != -1) {
		CString selectedText;
		m_combo.GetLBText(selectedIndex, selectedText);
		int spaceIndex = selectedText.Find(' ');
		CString ipAddress = selectedText.Left(spaceIndex);
		DisconnectClient(ipAddress);
	}
	else {
		AfxMessageBox(L"chose one");
	}
}

void CSocketServerDlg::OnEnChangeEdit2()
{
	// TODO:  如果這是 RICHEDIT 控制項，控制項將不會
	// 傳送此告知，除非您覆寫 CDialogEx::OnInitDialog()
	// 函式和呼叫 CRichEditCtrl().SetEventMask()
	// 讓具有 ENM_CHANGE 旗標 ORed 加入遮罩。

	// TODO:  在此加入控制項告知處理常式程式碼
}
