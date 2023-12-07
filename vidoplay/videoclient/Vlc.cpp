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
	if (m_instance == NULL)return -1;
	m_media=libvlc_media_new_path(m_instance,strUrl.c_str());
	if (m_media == NULL) return -2;
	m_palyer = libvlc_media_player_new_from_media(m_media);
	if (m_palyer == NULL) return -3;
	return 0;
}

int Vlc::SetHwnd(HWND hWnd)
{
	if (!m_palyer||!m_instance||!m_media) return -1;
	libvlc_media_player_set_hwnd(m_palyer, hWnd);
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
	if (!m_palyer || !m_instance || !m_media) return -1.0;
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
