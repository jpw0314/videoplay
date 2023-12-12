#pragma once
#include "Socket.h"
#include <map>
#include "Thread.h"
#include "Queue.h"

class RTSPRquest
{
public:
	RTSPRquest();
	RTSPRquest& operator=(const RTSPRquest& r);
	RTSPRquest(const RTSPRquest& r);
	~RTSPRquest();
private:
	int m_method;//0 OPTIONS 1DESCRIBE 2SETUP 3PLAY 4TEARDOWN
};

class RTSPResponse
{
public:
	RTSPResponse();
	~RTSPResponse();
	RTSPResponse& operator=(const RTSPResponse& r);
	RTSPResponse(const RTSPResponse& r);
	Buffer toBuffer();
private:
	int m_method;//0 OPTIONS 1DESCRIBE 2SETUP 3PLAY 4TEARDOWN
};

class RTSPSessions
{
public:
	RTSPSessions();

	~RTSPSessions();
	RTSPSessions& operator=(const RTSPSessions& r);
	RTSPSessions(const RTSPSessions& R);

};


class RTSPServer
{
public:
	RTSPServer():m_sock(),m_status(0),m_pool(10){}
	~RTSPServer();
	int Init(const std::string strip="0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
protected:
	int threadWorke(void* arg);
	RTSPRquest AnalyseRequest(const std::string& data);
	RTSPResponse MakeReply(const RTSPRquest& request);
	int ThreadSessions();
private:
	CSock m_sock;
	int m_status;//0 δ��ʼ�� 1��ʼ����� 2�������� 3�ر�
	CThread m_threadMain;
	ThreadPool m_pool;
	std::map < std::string,RTSPResponse> m_sessions;//�Ự
	EAddr m_addr;
	CQueue<CSock> m_client;
};

