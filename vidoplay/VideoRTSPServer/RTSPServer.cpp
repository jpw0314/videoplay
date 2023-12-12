#include "RTSPServer.h"

RTSPServer::~RTSPServer()
{
	Stop();
}

int RTSPServer::Init(const std::string strip, short port)
{
	m_addr.Update(strip, port);
	int ret=m_sock.Bind(m_addr);
	ret = m_sock.Listen();
	m_threadMain.UpdataWorker(ThreadWorker(this,(FUNCTYPE)RTSPServer::threadWorke));
	return 0;
}

int RTSPServer::Invoke()
{
	m_threadMain.Start();
	return 0;
}

void RTSPServer::Stop()
{
	m_sock.Close();
	m_threadMain.Stop();
	m_pool.Stop();
}

int RTSPServer::threadWorke(void* arg)
{
	EAddr client_addr;
	CSock client=m_sock.Accept(client_addr);
	if (client!=INVALID_SOCKET)
	{
		m_client.PushBack(client);
		m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::ThreadSessions));
	}

	return 0;
}

int RTSPServer::ThreadSessions()
{
	//TODO:接受请求，解析请求，应答请求
	CSock client;
	Buffer buffer(1024*16);
	int len=client.Recv(buffer);
	if (len<=0)
	{
		//
		return -1;
	}
	buffer.resize(len);
	RTSPRquest req=AnalyseRequest(buffer);
	RTSPResponse  ack = MakeReply(req);
	client.Send(ack.toBuffer());
	return 0;
}
