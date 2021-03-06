// ChatServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChatServer.h"
#include "ChatServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_SOCKET		WM_USER+1

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CChatServerDlg dialog




CChatServerDlg::CChatServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChatServerDlg::IDD, pParent)
	, m_szChat(_T(""))
	, m_szStatus(_T("Server đang tắt"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChatServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HISTORY, m_lstHistory);
	DDX_Text(pDX, IDC_CHAT, m_szChat);
	DDX_Text(pDX, IDC_STATUS, m_szStatus);
}

BEGIN_MESSAGE_MAP(CChatServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_START, &CChatServerDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_SEND, &CChatServerDlg::OnBnClickedSend)
	ON_BN_CLICKED(IDC_STOP, &CChatServerDlg::OnBnClickedStop)
END_MESSAGE_MAP()


// CChatServerDlg message handlers

BOOL CChatServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	WSADATA		wsaData;
	WORD		wVersion = MAKEWORD(2,2);
	WSAStartup(wVersion,&wsaData);
	nClient = 0;
	memset(clients,0,sizeof(SOCKET)*1024);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CChatServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CChatServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CChatServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CChatServerDlg::OnBnClickedStart()
{
	// TODO: Add your control notification handler code here
	server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	SOCKADDR_IN	serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8888);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int ret = bind(server,(sockaddr*)&serverAddr,
			sizeof(serverAddr));
	ret = listen(server,16);
	/*
	SOCKADDR_IN	clientAddr;
	int			clientAddrLen = sizeof(clientAddr);
	SOCKET		client;
	client = accept(server,(sockaddr*)&clientAddr,
		&clientAddrLen);
	*/
	
	WSAAsyncSelect(server,this->m_hWnd,WM_SOCKET,
		FD_ACCEPT);
	m_szStatus = L"Server đang đợi kết nối...";
	UpdateData(FALSE);

}

LRESULT CChatServerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	int i;
	SOCKADDR_IN	clientAddr;
	int			clientAddrLen = sizeof(clientAddr);
	wchar_t		szMsg[1024];
	int			len;
	wchar_t		szThongbao[1024];
	switch (message)
	{
	case WM_SOCKET:
		if (WSAGETSELECTERROR(lParam))
		{
			for (i = 0;i<nClient;i++)
				if (clients[i]==wParam)
				{
					closesocket(clients[i]);
					clients[i] = 0;
					break;
				};
			wsprintf(szMsg,L"Client %d ngắt kết nối !",i);
			MessageBox(szMsg,L"Lỗi !",MB_ICONERROR);
			break;
		}
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			for (i=0;i<nClient;i++)
				if (clients[i]==0) break;
			clients[i] = accept(server,
				(sockaddr*)&clientAddr,
				&clientAddrLen);
			wsprintf(szMsg,L"Có kết nối mới, cất vào vị trí %d",i);
			if (i==nClient) nClient++;
			MessageBox(szMsg,L"Thông báo",
				MB_ICONINFORMATION);
			WSAAsyncSelect(clients[i],this->m_hWnd,
				WM_SOCKET,FD_READ|FD_CLOSE|FD_WRITE);
			m_szStatus = L"Client đã kết nối !";
			UpdateData(FALSE);
			break;
		case FD_READ:
			len = recv(wParam,(char*)szMsg,2048,0);
			if (len > 0)
			{
				szMsg[len/2] = 0;
				// In ra cua so History
				for (i=0;i<nClient;i++)
					if (wParam==clients[i]) break;
				wsprintf(szThongbao,L"Client %d:%s",i,szMsg);
				m_lstHistory.AddString(szThongbao);
			};
			break;
		case FD_WRITE:
			break;

		}






	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void CChatServerDlg::OnBnClickedSend()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	for (int i=0;i<nClient;i++)
		if (clients[i]!=0)
		{
			send(clients[i],(char*)m_szChat.GetBuffer(),
				m_szChat.GetLength()*2,0);
		}
}

void CChatServerDlg::OnBnClickedStop()
{
	// TODO: Add your control notification handler code here
	closesocket(server);
	for (int i=0;i<nClient;i++)
		if (clients[i]!=0)
			closesocket(clients[i]);
	memset(clients,0,sizeof(SOCKET)*1024);
	nClient = 0;
	m_szStatus = L"Server đang tắt";
	UpdateData(FALSE);
}
