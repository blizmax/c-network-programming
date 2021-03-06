// ChatClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChatClient.h"
#include "ChatClientDlg.h"

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


// CChatClientDlg dialog




CChatClientDlg::CChatClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChatClientDlg::IDD, pParent)
	, m_szChat(_T(""))
	, m_szAddress(_T("Địa chỉ IP / Tên miền"))
	, m_szPort(_T("Cổng"))
	, m_szNickname(_T("Nickname"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChatClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CHAT, m_szChat);
	DDX_Text(pDX, IDC_ADD, m_szAddress);
	DDX_Text(pDX, IDC_PORT, m_szPort);
	DDX_Text(pDX, IDC_NICK, m_szNickname);
	DDX_Control(pDX, IDC_HISTORY, m_History);
}

BEGIN_MESSAGE_MAP(CChatClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CONNECT, &CChatClientDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_SEND, &CChatClientDlg::OnBnClickedSend)
	ON_BN_CLICKED(IDC_LOGIN, &CChatClientDlg::OnBnClickedLogin)
END_MESSAGE_MAP()


// CChatClientDlg message handlers

BOOL CChatClientDlg::OnInitDialog()
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

	WSADATA	wsaData;
	WORD	wVersion = MAKEWORD(2,2);
	WSAStartup(wVersion,&wsaData);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CChatClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CChatClientDlg::OnPaint()
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
HCURSOR CChatClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CChatClientDlg::OnBnClickedConnect()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	client = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	SOCKADDR_IN	serverAddr;
	
	CString m_szAdd = m_szAddress;
	char m_szHostname[1024];
	WideCharToMultiByte(CP_ACP, 0,m_szAdd, m_szAdd.GetLength(), m_szHostname, 1024, 0, 0);
	m_szHostname[m_szAdd.GetLength()] = 0;
	
	CString m_szP = m_szPort;
	char m_szCong[1024];
	WideCharToMultiByte(CP_ACP, 0, m_szP, m_szP.GetLength(), m_szCong, 1024, 0, 0);
	m_szCong[m_szP.GetLength()] = 0;


	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(m_szCong));
	serverAddr.sin_addr.s_addr = inet_addr(m_szHostname);
	WSAAsyncSelect(client,this->m_hWnd,
		WM_SOCKET,FD_READ|FD_CONNECT|FD_CLOSE);
	connect(client,(sockaddr*)&serverAddr,
		sizeof(serverAddr));
}

LRESULT CChatClientDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	int len;
	wchar_t szMsg[1024];
	wchar_t szThongbao[1024];
	switch (message)
	{
	case WM_SOCKET:
		if (WSAGETSELECTERROR(lParam))
		{
			MessageBox(L"Lỗi kết nối !",
				L"Lỗi",MB_ICONERROR
				);
			break;
		};
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_CONNECT:
			MessageBox(L"Đã kết nối đến server",
				L"Thông báo",MB_ICONINFORMATION);
			break;
		case FD_READ:
			len = recv(wParam, (char*)szMsg, 2048, 0);
			if (len > 0)
			{
				szMsg[len / 2] = 0;
				// In ra cua so History
				wsprintf(szThongbao, L"Server: %s", szMsg);
				m_History.AddString(szThongbao);
			};
			break;
		case FD_WRITE:
			break;
		}

	}
	return CDialog::WindowProc(message, wParam, lParam);
}

void CChatClientDlg::OnBnClickedSend()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	send(client,(char*)m_szChat.GetBuffer(),
		m_szChat.GetLength()*2,0);
	m_History.AddString(m_szChat);
}


void CChatClientDlg::OnBnClickedLogin()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	send(client, (char*)m_szNickname.GetBuffer(),
		m_szNickname.GetLength() * 2, 0);
}

