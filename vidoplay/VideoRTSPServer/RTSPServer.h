#pragma once
#include "Socket.h"
#include <map>
#include "Thread.h"
#include "Queue.h"
#include "RTPHelper.h"
#include "MediaFile.h"

class RTSPSessions;
class RTSPServer;
typedef void(*RTPPLAYCB)(RTSPServer* thiz, RTSPSessions& session);

class RTSPRquest
{
public:
	RTSPRquest();
	RTSPRquest& operator=(const RTSPRquest& protocol);
	RTSPRquest(const RTSPRquest& protocol);
	~RTSPRquest();
	void SetMethod(const Buffer& method);
	void SetUrl(const Buffer& url);
	void SetSequence(const Buffer& seq);
	void SetClientPort(int ports[]);
	void SetSession(const Buffer& session);
	const Buffer& url()const { return m_url; }
	int method()const { return m_method; }
	const Buffer& session()const { return m_session; }
	const Buffer& sequence()const { return m_seq; }
	Buffer port(int index = 0)const { return index?m_client_port[1] : m_client_port[0]; }
private:
	int m_method;//0 OPTIONS 1DESCRIBE 2SETUP 3PLAY 4TEARDOWN
	Buffer m_url;
	Buffer m_session;
	Buffer m_seq;
	Buffer m_client_port[2];
};

class RTSPResponse
{
public:
	RTSPResponse();
	~RTSPResponse();
	RTSPResponse& operator=(const RTSPResponse& protocol);
	RTSPResponse(const RTSPResponse& protocol);
	Buffer toBuffer();
	void SetOptins(const Buffer& options);
	void SetSequence(const Buffer& seq);
	void SetSdp(const Buffer& sdp);
	void SetClientPort(const Buffer& port0, const Buffer& port1);
	void SetServerPort(const Buffer& port0, const Buffer& port1);
	void SetSession(const Buffer& session);
	void SetMethod(int medthod);

private:
	int m_method;//0 OPTIONS 1DESCRIBE 2SETUP 3PLAY 4TEARDOWN
	unsigned short m_server_port[2];
	unsigned short m_client_port[2];
	Buffer m_sdp;
	Buffer m_options;
	Buffer m_session;
	Buffer m_seq;
};

class RTSPSessions
{
public:
	RTSPSessions();
	RTSPSessions(const CSock& client);
	~RTSPSessions();
	RTSPSessions& operator=(const RTSPSessions& protocol);
	RTSPSessions(const RTSPSessions& protocol);
	int SessionMainThing(RTPPLAYCB cb,RTSPServer* thiz);
	EAddr GetClientUdpAddr()const;
private:
	Buffer GetOneLine( Buffer& buf);
	Buffer GetRequest();
	RTSPRquest AnalyseRequest(const Buffer& buffer);
	RTSPResponse Reply(const RTSPRquest& request); 
private:
	CSock m_client;
	std::string m_id;
	short m_port;
};


class RTSPServer :public ThreadFuncBase
{
public:
	RTSPServer():m_sock(),m_status(0),m_pool(10){
		m_threadMain.UpdataWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::threadWorke	));
		m_h264.Open("C:\\Users\\peng\\Desktop\\123\\test.h264",96);
	}
	~RTSPServer();
	int Init(const std::string strip="0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
protected:
	int threadWorke();
	int ThreadSessions();
	void UdpWorker(const EAddr& client);
	static void PlayCallBack(RTSPServer* thiz, RTSPSessions& session);
private:
	CSock m_sock;
	int m_status;//0 未初始化 1初始化完成 2正在运行 3关闭
	CThread m_threadMain;
	ThreadPool m_pool;
	std::map < std::string,RTSPResponse> m_sessions;//会话
	EAddr m_addr;
	CQueue<RTSPSessions> m_listSessions;
	RTPHelper m_helper;
	CMediaFile m_h264;
};

