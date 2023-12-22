#include "pch.h"
#include "Vlc.h"

Vlc::~Vlc()
{
	if (m_instance != NULL)
	{
		libvlc_media_player_t* temp = m_palyer;
		m_palyer = NULL;
		libvlc_media_player_release(temp);
	}

	if (m_media != NULL)
	{
		libvlc_media_t* temp = m_media;
		m_media = NULL;
		libvlc_media_release(temp);
	}

	if (m_instance != NULL)
	{
		libvlc_instance_t* temp = m_instance;
		m_instance = NULL;
		libvlc_release(temp);
	}
}

int Vlc::SetMedia(std::string strUrl)
{
	if ((m_instance == NULL)||(m_hWnd==NULL))return -1;
	if (m_url==strUrl)return 0;
	m_url = strUrl;
	if (m_media != NULL)
	{
		libvlc_media_release(m_media);
		m_media = NULL;
	}
	m_media=libvlc_media_new_location(m_instance,strUrl.c_str());
	if (m_media == NULL) return -2;
	if (m_palyer != NULL)
	{
		libvlc_media_player_release(m_palyer);
		m_palyer = NULL;
	}
	m_palyer = libvlc_media_player_new_from_media(m_media);
	if (m_palyer == NULL) return -3;
	libvlc_media_player_set_hwnd(m_palyer, m_hWnd );
	return 0;
}

int Vlc::SetHwnd(HWND hWnd)
{
	m_hWnd = hWnd;
	return 0;
}

int Vlc::Play()
{
	if (!m_palyer || !m_instance || !m_media) return -1;
	return libvlc_media_player_play(m_palyer);
}

int Vlc::Pause()
{
	if (!m_palyer || !m_instance || !m_media) return -1;
	libvlc_media_player_pause(m_palyer);
	return 0;
}

int Vlc::Stop()
{
	if (!m_palyer || !m_instance || !m_media) return -1;
	libvlc_media_player_stop(m_palyer);
	return 0;
}

float Vlc::GetPosition()
{
	if (!m_palyer || !m_instance || !m_media) return -1.0f;
	return libvlc_media_player_get_position(m_palyer);
}

void Vlc::SetPosition(float pos)
{
	if (!m_palyer || !m_instance || !m_media) return;
	libvlc_media_player_set_position(m_palyer, pos);
}

int Vlc::GetVolume()
{
	if (!m_palyer || !m_instance || !m_media) return -1;
	return libvlc_audio_get_volume(m_palyer);
}

float Vlc::Getlength()
{
	if (!m_palyer || !m_instance || !m_media) return -1.0f;
	libvlc_time_t tm=libvlc_media_player_get_length(m_palyer);
	float ret = tm / 1000.0f;
	return ret;
}

int Vlc::SetVloume(int volume)
{
	if (!m_palyer || !m_instance || !m_media) return -1;
	return libvlc_audio_set_volume(m_palyer, volume);
}

VlcSize Vlc::GetMediaInfo()
{
	if (!m_palyer || !m_instance || !m_media) VlcSize(-1,-1);
	return VlcSize(libvlc_video_get_width(m_palyer),libvlc_video_get_height(m_palyer));
}

std::string Vlc::unicode2utf8(const std::wstring& strIn)
{
	std::string str;
	int length = ::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), LPSTR(str.c_str()), 0, NULL, NULL);
	str.resize(length + 1);
	::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), LPSTR(str.c_str()), length, NULL, NULL);
	return str;
}
