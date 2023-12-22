
// videoclientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "videoclient.h"
#include "videoclientDlg.h"
#include "afxdialogex.h"
#include "VideoClientController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CvideoclientDlg 对话框



CvideoclientDlg::CvideoclientDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_VIDEOCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_status = false;
	m_length = 0.0f;
}

void CvideoclientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PLAY, m_video);
	DDX_Control(pDX, IDC_SLIDER_POS, m_pos);
	DDX_Control(pDX, IDC_SLIDER_VOLUME, m_volume);
	DDX_Control(pDX, IDC_EDIT_URL, m_url);
	DDX_Control(pDX, IDC_BTN_PLAY, m_btnplay);
}

BEGIN_MESSAGE_MAP(CvideoclientDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_PLAY, &CvideoclientDlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_STOP, &CvideoclientDlg::OnBnClickedBtnStop)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_POS, &CvideoclientDlg::OnTRBNThumbPosChangingSliderPos)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_VOLUME, &CvideoclientDlg::OnTRBNThumbPosChangingSliderVolume)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


// CvideoclientDlg 消息处理程序

BOOL CvideoclientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	SetTimer(0, 500, NULL);
	m_pos.SetRange(0, 1); 
	m_volume.SetRange(0, 100);
	m_volume.SetTicFreq(20);
	SetDlgItemText(IDC_STATIC_VOLUME, _T("100%"));
	SetDlgItemText(IDC_STATIC_TIME, _T("--:--:--/--:--:--"));
	m_controllerl->SetHwnd(m_video.GetSafeHwnd());
	//m_url.SetWindowText( _T("file:///C:\\Users\\peng\\Desktop\\我的文件\\QQ20231214-174442.mp4"));
	m_url.SetWindowText(_T("rtsp://127.0.0.1:554"));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CvideoclientDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CvideoclientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CvideoclientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值	
	if (nIDEvent==0)
	{
		//控制层
		float pos=m_controllerl->videoctrl(VLC_GET_POSITION);
		if (pos!=-1.0)
		{
			if(m_length==0.0f) m_length= m_controllerl->videoctrl(VLC_GET_LENGTH);
			m_pos.SetRange(0, int(m_length));
			CString strpos;
			strpos.Format(_T("%0.3f/%0.3f"), pos*m_length,m_length);
			SetDlgItemText(IDC_STATIC_TIME, strpos);
			m_pos.SetPos(int(m_length * pos));
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CvideoclientDlg::OnDestroy()
{
	CDialog::OnDestroy();
	KillTimer(0);
	// TODO: 在此处添加消息处理程序代码
}


void CvideoclientDlg::OnBnClickedBtnPlay()
{
	if (m_status==false)
	{
		CString url;
		m_url.GetWindowText(url);
		m_controllerl->SetMedia(m_controllerl->unicode2utf8((LPCTSTR)url));
		m_btnplay.SetWindowText(_T("暂停"));
		m_status = true;
		m_controllerl->videoctrl(VLC_PLAY);
	}
	else {
		m_btnplay.SetWindowTextW(_T("播放"));
		m_status = false;
		m_controllerl->videoctrl(VLC_PAUSE);
	}
}


void CvideoclientDlg::OnBnClickedBtnStop()
{
	m_btnplay.SetWindowText(_T("播放"));
	m_status = false;
	m_controllerl->videoctrl(VLC_STOP);
}


void CvideoclientDlg::OnTRBNThumbPosChangingSliderPos(NMHDR* pNMHDR, LRESULT* pResult)
{
	// 此功能要求 Windows Vista 或更高版本。
	// _WIN32_WINNT 符号必须 >= 0x0600。
	NMTRBTHUMBPOSCHANGING* pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	TRACE("pos=%d  reason=%d \n", pNMTPC->dwPos, pNMTPC->nReason);
	*pResult = 0;
}


void CvideoclientDlg::OnTRBNThumbPosChangingSliderVolume(NMHDR* pNMHDR, LRESULT* pResult)
{
	// 此功能要求 Windows Vista 或更高版本。
	// _WIN32_WINNT 符号必须 >= 0x0600。
	NMTRBTHUMBPOSCHANGING* pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	TRACE("pos=%d  reason=%d \n", pNMTPC->dwPos, pNMTPC->nReason);
	*pResult = 0;
}


void CvideoclientDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	TRACE("m_pos=%p m_volume=%p pScrollBar%p nSBCode=%d\n", &m_pos, &m_volume, pScrollBar, nSBCode);
	if (nSBCode == 5)
	{
		CString strVolume;
		strVolume.Format(_T("%d%%"), (nPos));
		SetDlgItemText(IDC_STATIC_TIME, strVolume);
		m_controllerl->SetPosition((float)nPos/m_length);
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CvideoclientDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	TRACE("m_pos=%p m_volume=%p pScrollBar%p nSBCode=%d\n", &m_pos, &m_volume, pScrollBar,nSBCode);

	if (nSBCode==5)
	{
		CString strVolume;
		strVolume.Format(_T("%d%%"), (100 - nPos));
		SetDlgItemText(IDC_STATIC_VOLUME, strVolume);
		m_controllerl->SetVolume(100-nPos);
	}
	
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


