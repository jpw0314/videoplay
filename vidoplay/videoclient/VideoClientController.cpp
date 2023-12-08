#include "pch.h"
#include "VideoClientController.h"

CVideoClientController::CVideoClientController()
{
	m_dlg.m_controllerl = this;
}

CVideoClientController::~CVideoClientController()
{
}

int CVideoClientController::Init(CWnd* pWnd)
{
    pWnd = &m_dlg;
	return 0;
    
}

int CVideoClientController::Invoke()
{
	INT_PTR nResponse = m_dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}
	return nResponse;
}

float CVideoClientController::videoctrl(vlcCmd cmd)
{
	switch (cmd)
	{
	case VLC_PLAY:
		return (float)m_vlc.Play();
	case VLC_PAUSE:
		return (float)m_vlc.Pause();
	case VLC_STOP:
		return (float)m_vlc.Stop();
	case VLC_GET_VOLUME:
		return (float)m_vlc.GetVolume();
	case VLC_GET_POSITION:
		return m_vlc.GetPosition();
	default:
		return -1.0;
		break;
	}
	return -1.0;
}

void CVideoClientController::SetPosition(float pos)
{
	m_vlc.SetPosition(pos);
}

int CVideoClientController::SetVolume(int volume)
{
	return m_vlc.SetVloume(volume);
}

float CVideoClientController::GetPosition()
{
	return m_vlc.GetPosition();
}

int CVideoClientController::GetVolume()
{
	return m_vlc.GetVolume();
}

void CVideoClientController::SetMedia(const std::string strUrl)
{
	m_vlc.SetMedia(strUrl);
}

void CVideoClientController::SetHwnd(HWND hWnd)
{
	m_vlc.SetHwnd(hWnd);
}

VlcSize CVideoClientController::GetMediaInfo()
{
	return m_vlc.GetMediaInfo();
}

std::string CVideoClientController::unicode2utf8(const std::wstring& strIn)
{
	return m_vlc.unicode2utf8(strIn);
}

