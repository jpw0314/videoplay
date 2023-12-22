#include "RTPHelper.h"
#include <Windows.h>

#define  RTP_MAX_SIZE 1300
int RTPHelper::SendMediaFrame(RTPFrame& rtpframe,Buffer& frame, const EAddr& client)
{
    size_t frame_size = frame.size();
    int sepsize=GetFrameSepSize(frame);
    frame_size -= sepsize;
    BYTE* pFrame = sepsize + (BYTE*)frame;
    if (frame_size > RTP_MAX_SIZE)
    {
        size_t restsize = frame_size % RTP_MAX_SIZE;
        int count = frame_size / RTP_MAX_SIZE;
        for (int i=0;i<count;i++)
        {
            rtpframe.m_load.resize(RTP_MAX_SIZE+2);
            ((BYTE*)rtpframe.m_load)[0] = 0x60 | 28;//(0110 0000=0x60) 28=0x1c 0111 1100
            ((BYTE*)rtpframe.m_load)[1] =(pFrame[0]&0x1F);//中间 0000 0000 | 0001 1111
            if (i==0)
				((BYTE*)rtpframe.m_load)[1] = 0x80|((BYTE*)rtpframe.m_load)[1];//开始1000 0000 | 0001 1111
            else if ((restsize==0)&&(count-1==i))
                ((BYTE*)rtpframe.m_load)[1] = 0x40|((BYTE*)rtpframe.m_load)[1];//结束 0100 0000 | 0001 1111
                memcpy(2 + (BYTE*)rtpframe.m_load, pFrame+(i*RTP_MAX_SIZE)+1, RTP_MAX_SIZE);
                //TODO 发包
                SendFrame(rtpframe, client);
                rtpframe.m_head.serial++; 
        }

        if (restsize>0)
        {
            rtpframe.m_load.resize(restsize + 2);
			((BYTE*)rtpframe.m_load)[0] = 0x60 | 28;//(0110 0000=0x60) 28=0x1c 0111 1100
			((BYTE*)rtpframe.m_load)[1] = (pFrame[0] & 0x1F);//中间 0000 0000 | 0001 1111
            ((BYTE*)rtpframe.m_load)[1] = 0x40 | (*(pFrame) & 0x1F);//结束 0100 0000 | 0001 1111
            memcpy(2+(BYTE*)rtpframe.m_load, pFrame + RTP_MAX_SIZE * count+1, restsize);
            SendFrame(rtpframe, client);
            rtpframe.m_head.serial++;
        }
    }
    else{//小包
    
        rtpframe.m_load.resize(frame.size() - sepsize);
        memcpy(rtpframe.m_load, pFrame,frame.size() - sepsize);
     
        //TODO处理RTP的header
        //序号是累加的，时间戳是计算出来的，从0开始每帧追加 时钟频率90000/每秒帧数24
        SendFrame(rtpframe, client);
        rtpframe.m_head.serial++;
    }
    rtpframe.m_head.timestamp += 90000 / 24;
    //序号是累加的，时间戳是计算出来的，从0开始每帧追加 时钟频率90000/每秒帧数24
    Sleep(1000 / 30);
    return 0;
}
  
int RTPHelper::GetFrameSepSize(const Buffer& frame)
{
    char buf[] = { 0,0,0,1 };
    if (memcmp(frame.c_str(), buf, 4) == 0)return 4;
    return 3;
}

int RTPHelper::SendFrame(const Buffer& frame,const EAddr& client)
{
    int ret = sendto(m_udp,frame,frame.size(),0,client,client.size());
	if (ret == SOCKET_ERROR) {
        wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
	}
    printf("ret=%d  size=%d ip=%s port=%d\n", ret, frame.size(), client.IP().c_str(),client.port());
    return 0;
}

RTPHeader::RTPHeader()
{
    csrccount = 0;
    extension = 0;
    padding = 0;
    version = 2;
    LoadType = 96;
    mark = 0;
    serial =0;
    timestamp = 0;
    ssrc = 0x98765432;
    memset(&csrc, 0, sizeof(csrc));
}

RTPHeader::RTPHeader(const RTPHeader& head)
{
    memset(&csrc, 0, sizeof(csrc));
    int size = 12 + 4 * csrccount;
    memcpy(this,&head, size);
}

RTPHeader& RTPHeader::operator=(const RTPHeader& head)
{  
    if (this!=&head)
    {
		int size = 12 + 4 * csrccount;
		memcpy(this, &head, size);
    }
    return *this;
}

RTPHeader::operator Buffer()const
{
    RTPHeader head = *this;
    head.timestamp=htonl(head.timestamp);
    head.serial = htons(head.serial);
    head.ssrc = htonl(head.ssrc);
    int size = 12 + 4 * csrccount;
    Buffer result(size);
    memcpy(result,&head, size);
    return result;
}

RTPFrame::operator Buffer()
{
    Buffer result="";
    result += (const Buffer)m_head;
    result +=m_load;
    return result;
}
