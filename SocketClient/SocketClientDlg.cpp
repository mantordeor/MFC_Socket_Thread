// SocketClientDlg.cpp: 實作檔案
//

#include "pch.h"
#include "framework.h"
#include "SocketClient.h"
#include "SocketClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
typedef struct {
	SOCKET ClientSocket;
	SOCKADDR_IN ServerAddr;
} ConnectionInfo;

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

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSocketClientDlg 對話方塊



CSocketClientDlg::CSocketClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SOCKETCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSocketClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSocketClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CSocketClientDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CSocketClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CSocketClientDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

typedef struct {
	SOCKET clientSocket;
	SOCKADDR_IN serverAddr;
	int threadId; // 添加线程ID
} ThreadInfo;

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
	filename.Format(_T("Client_log_%s.txt"), currentDate);
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

DWORD WINAPI SendMessages(LPVOID lpParam) {
	ThreadInfo* threadInfo = (ThreadInfo*)lpParam;
	SOCKET clientSocket = threadInfo->clientSocket;
	SOCKADDR_IN serverAddr = threadInfo->serverAddr;
	int threadId = threadInfo->threadId;
	int r = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof serverAddr);
	if (r == -1) {

        CString msg;
        msg.Format(_T("Thread %d: Connecting Server Failed: %u"), threadId, GetLastError());
		LogAction(msg);
        AfxMessageBox(msg);
		closesocket(clientSocket);
		return 1;
	}

	char buff[1024] = "123456";
	for(int i = 0 ; i < 10 ; i++) {
		r = send(clientSocket, buff, strlen(buff), 0);
		if (r == SOCKET_ERROR) {
			CString msg;
			msg.Format(_T("Thread % d: Send failed : % u"), threadId, GetLastError());
			LogAction(msg);
			AfxMessageBox(msg);
			break;
		}
		Sleep(1000);
	}

	closesocket(clientSocket);
	return 0;
}

// CSocketClientDlg 訊息處理常式
BOOL CSocketClientDlg::OnInitDialog()
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

void CSocketClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
// 以便繪製圖示。對於使用文件/檢-view模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CSocketClientDlg::OnPaint()
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
HCURSOR CSocketClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSocketClientDlg::OnBnClickedCancel()
{
	closesocket(ClientSocket);
	WSACleanup();
	// TODO: 在此加入控制項告知處理常式程式碼
	CDialogEx::OnCancel();
}

BOOL CSocketClientDlg::PreTranslateMessage(MSG* pMsg) {
	if (pMsg->message == WM_KEYDOWN || pMsg->message == VK_ESCAPE) return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}

void CSocketClientDlg::OnBnClickedOk()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	//CDialogEx::OnOK();
}

void CSocketClientDlg::OnBnClickedButton1()
{
	HANDLE hThreads[10];
	ThreadInfo* threadInfo[10];
	// TODO: 在此加入控制項告知處理常式程式碼
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (HIBYTE(wsaData.wVersion) != 2 || LOBYTE(wsaData.wVersion) != 2) return ;
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == -1) {
		CString msg;
		msg.Format(_T("CreateSocket failed : % u"), GetLastError());
		LogAction(msg);
		AfxMessageBox(msg);
		WSACleanup();
		return;
	}
	SOCKADDR_IN ServerAddr = { 0 };
	ServerAddr.sin_family = AF_INET;
	InetPton(AF_INET, _T("127.0.0.1"), &ServerAddr.sin_addr.S_un.S_addr);
	ServerAddr.sin_port = htons(7000);
	int r = connect(ClientSocket, (const sockaddr*)&ServerAddr, sizeof ServerAddr);
	if (r == -1) {
		CString msg;
		msg.Format(_T("ConnectSocket failed : % u"), GetLastError());
		LogAction(msg);
		AfxMessageBox(msg);
		WSACleanup();
		return ;
	}
	for (int i = 0; i < 10; ++i) {
		SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (clientSocket == -1) {
			CString msg;
			msg.Format(_T("Thread %d: Create socket failed: %u\n"),i, GetLastError());
			LogAction(msg);
			AfxMessageBox(msg);
			WSACleanup();
			return;
		}
		SOCKADDR_IN serverAddr = { 0 };
		serverAddr.sin_family = AF_INET;
		InetPton(AF_INET, _T("127.0.0.1"), &serverAddr.sin_addr.S_un.S_addr);
		serverAddr.sin_port = htons(7000);

		threadInfo[i] = (ThreadInfo*)malloc(sizeof(ThreadInfo));
		threadInfo[i]->clientSocket = clientSocket;
		threadInfo[i]->serverAddr = serverAddr;
		threadInfo[i]->threadId = i;

		// 建立執行緒
		hThreads[i] = CreateThread(NULL, 0, SendMessages, threadInfo[i], 0, NULL);
	}
	WaitForMultipleObjects(10, hThreads, TRUE, INFINITE);

	// 清理資源
	for (int i = 0; i < 10; ++i) {
		CloseHandle(hThreads[i]);
		
	}
	for (int j = 0; j < 10; ++j) {
		closesocket(threadInfo[j]->clientSocket);
		free(threadInfo[j]);
	}
}
