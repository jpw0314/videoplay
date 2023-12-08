#pragma once
#include "videoclientDlg.h"
#include "Vlc.h"

enum vlcCmd
{
	VLC_PLAY,
	VLC_PAUSE,
	VLC_STOP,
	VLC_GET_VOLUME,
	VLC_GET_POSITION
};

class CVideoClientController
{
public:
	CVideoClientController();
	~CVideoClientController();

	int Init(CWnd* pWnd);
	int Invoke();
	float videoctrl(vlcCmd cmd);
	void SetPosition(float pos);
	int SetVolume(int volume);
	float GetPosition();
	int GetVolume();
	void SetMedia(const std::string strUrl);
	void SetHwnd(HWND hWnd);
	VlcSize GetMediaInfo();
	std::string unicode2utf8(const std::wstring& strIn);
private:
	CvideoclientDlg m_dlg;
	Vlc m_vlc;
};

