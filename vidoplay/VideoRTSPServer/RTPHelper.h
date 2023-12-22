#pragma once
#include "Tool.h"
#include "Socket.h"

class RTPHeader
{
public:
	unsigned short csrccount : 4;
	unsigned short extension : 1;
	unsigned short padding : 1;
	unsigned short version : 2;
	unsigned short LoadType : 7;//∫…‘ÿ
	unsigned short mark : 1;//±Íº«
	unsigned short serial;//–Ú¡–
	unsigned int timestamp;
	unsigned int ssrc;
	unsigned int csrc[15] ;
public:
	RTPHeader();
	RTPHeader(const RTPHeader& head);
	RTPHeader& operator=(const RTPHeader& head);
	operator Buffer()const;
};

class RTPFrame
{
public:
	RTPHeader m_head;
	Buffer m_load;
	operator Buffer();
};


class RTPHelper
{
public:
	RTPHelper():timestamp(0),m_udp(false){
		m_udp.Bind(EAddr("0.0.0.0", 55000));
	}
	~RTPHelper(){
	}
	int SendMediaFrame(RTPFrame& rtpframe,Buffer& frame, const EAddr& client);
private:
	int GetFrameSepSize(const Buffer& frame);
	int SendFrame(const Buffer& frame,const EAddr& client);
	DWORD timestamp;
	CSock m_udp;
};

