#pragma once
#include <vlc.h>
#include <string>

class VlcSize
{
public:
	int nWidth;
	int nHeight;

	VlcSize(int width=0,int height=0)
	{
		nWidth = width;
		nHeight = height;
	}

	VlcSize(const VlcSize& size)
	{
		nHeight = size.nHeight;
		nWidth = size.nWidth;
	}

	VlcSize& operator=(const VlcSize& size)
	{
		if (this != &size)
		{
			nHeight = size.nHeight;
			nWidth = size.nWidth;
		}
		return *this;
	}
};


class Vlc
{
public:
	Vlc()
	{
		m_instance = libvlc_new(0, NULL);
		m_media = NULL;
		m_palyer = NULL;

	}
	~Vlc();
public:
	int SetMedia(std::string strUrl);
	int SetHwnd(HWND hWnd);
	int Play();
	int Pause();
	int Stop();
	float GetPosition();
	void SetPosition(float pos);
	int GetVolume();
	int SetVloume(int volume);
	VlcSize GetMediaInfo();
protected:
	libvlc_instance_t* m_instance;
	libvlc_media_t* m_media;
	libvlc_media_player_t* m_palyer;
};

