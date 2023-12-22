#include "RTSPServer.h"
#include <rpc.h>
#include "Tool.h"

#pragma warning(disable:6031)
#pragma  comment(lib,"rpcrt4.lib")

RTSPServer::~RTSPServer()
{
	Stop();
}

int RTSPServer::Init(const std::string strip, short port)
{
	m_addr.Update(strip, port);
	int ret=m_sock.Bind(m_addr);
	ret = m_sock.Listen();
	m_threadMain.UpdataWorker(ThreadWorker(this,(FUNCTYPE)&RTSPServer::threadWorke));
	return 0;
}

int RTSPServer::Invoke()
{  
	m_threadMain.Start();
	m_pool.Invoke();
	return 0;
}

void RTSPServer::Stop()
{
	m_sock.Close();
	m_threadMain.Stop();
	m_pool.Stop();
}

int RTSPServer::threadWorke()
{
	EAddr client_addr;
	CSock client=m_sock.Accept(client_addr);
	if (client!=INVALID_SOCKET)
	{
		RTSPSessions session(client);
		m_listSessions.PushBack(session);
		m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::ThreadSessions));
	}
	return 0;
}

int RTSPServer::ThreadSessions()
{
	//TODO:接受请求，解析请求，应答请求
	RTSPSessions session;
	if (m_listSessions.PopFront(session))
	{
		int ret=session.SessionMainThing(&RTSPServer::PlayCallBack,this);
		return ret;
	}
	return -1;
}

void RTSPServer::UdpWorker(const EAddr& client)
{
	Buffer frame = m_h264.ReadOneframe();
	RTPFrame rtp;
	while (frame.size()>0)
	{
		m_helper.SendMediaFrame(rtp, frame, client);
		frame = m_h264.ReadOneframe();
	}
}

void RTSPServer::PlayCallBack(RTSPServer* thiz, RTSPSessions& session)
{
	thiz->UdpWorker(session.GetClientUdpAddr());
}

RTSPSessions::RTSPSessions()
{
	UUID uid;
	UuidCreate(&uid);
	m_id.resize(10);
	snprintf((char*)m_id.c_str(), m_id.size(), "%u%u", uid.Data1,uid.Data2);
}

RTSPSessions::RTSPSessions(const CSock& client):m_client(client)
{
	//:m_client(client)
	//保证生成唯一的session id
	UUID uid;
	UuidCreate(&uid);
	m_id.resize(10);
	snprintf((char*)m_id.c_str(), m_id.size(),"%u%u", uid.Data1, uid.Data2);

}

RTSPSessions::~RTSPSessions()
{
}

RTSPSessions& RTSPSessions::operator=(const RTSPSessions& protocol)
{
	if (this!=&protocol)
	{
		m_id = protocol.m_id;
		m_client = protocol.m_client;
	}
	return *this;

}

RTSPSessions::RTSPSessions(const RTSPSessions& protocol)
{
	m_id = protocol.m_id;
	m_client = protocol.m_client;
}

int RTSPSessions::SessionMainThing(RTPPLAYCB cb, RTSPServer* thiz)
{
	int ret = -1; 
	do {
		Buffer buffer = GetRequest();
		if (buffer.size() <= 0) return -1;
		RTSPRquest req = AnalyseRequest(buffer);
		if (req.method() < 0) return -2;
		RTSPResponse  ack = Reply(req);
		ret = m_client.Send(ack.toBuffer());
		if (req.method()==2)
		{
			m_port = (short)atoi(req.port());
		}
		if (req.method() == 3)
		{
			cb(thiz, *this);
		}
	} while (ret>=0);
	if (ret < 0) return ret;
	return 0;
}

Buffer RTSPSessions::GetOneLine(Buffer& buf)
{
	int i = 0;
	if (buf.size() <= 0) return Buffer();
	Buffer result,temp;
	for (; i < (int)buf.size(); i++)
	{
		result += buf.at(i);
		if (buf.at(i)=='\n') break;
	}
	temp = (char*)buf + i+1; 
	buf = temp;
	return result;
}

Buffer RTSPSessions::GetRequest()
{
	Buffer buffer(1);
	Buffer result;
	int len = 1; 
	while (len>0)
	{
		buffer.Zero();
		len= m_client.Recv(buffer);
		if (len > 0)
		{
			result += buffer;
			if (result.size()>=4)
			{
				UINT val = *(UINT*)(result.size()-4 + (char*)result);
				if (val == *(UINT*)"\r\n\r\n")
				{
					break;
				}
			}
		}
	}
	return result;
}

RTSPRquest RTSPSessions::AnalyseRequest(const Buffer& buffer)
{
	TRACE("recv:<%s>\n", (char*)buffer);
	Buffer data = buffer;
	RTSPRquest request;
	Buffer line = GetOneLine(data);
	Buffer method(32), url(1024), version(16),seq(64);
	if ((sscanf(line, "%s %s %s \r\n", (char*)method, (char*)url, (char*)version)) != 3)
	{
		TRACE("Error at:[%s]\n",(char*)line);
		return request;
	}
	line = GetOneLine(data);
	if (sscanf(line, "CSeq: %s\r\n", (char*)seq) < 1)
	{
		TRACE("Error  at:[%s]\n",(char*)line);
		return request;
	}
	request.SetMethod(method);
	request.SetUrl(url);
	request.SetSequence(seq);
	if ((strcmp(method,"OPTIONS")==0)|| (strcmp(method,"DESCRIBE") == 0))
	{
		return request;
	} 
	else if (strcmp(method, "SETUP") == 0)
	{
		do {
			line = GetOneLine(data);
			if (strstr((const char*)line, "client_port=") == NULL)continue;
			break;
		} while (line.size() > 0);
		int port[2] = { 0,0 };	
		if (sscanf(line.c_str(), "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n",port,(port+1)) == 2)
		{
			request.SetClientPort(port);
			return request;
		}
	}else if ((strcmp(method, "PLAY") == 0)|| (strcmp(method,"TEARDOWN") == 0))
	{
		line = GetOneLine(data);
		Buffer session(64);
		if (sscanf(line, "Session: %s\r\n", (char*)session) == 1)
		{
			request.SetSession(session);
			return request;
		}
	
	}
	return request;
}

RTSPResponse RTSPSessions::Reply(const RTSPRquest& request)
{
	RTSPResponse reply;
	reply.SetMethod(request.method());
	reply.SetSequence(request.sequence());
	if (request.session().size()>0)
	{
		reply.SetSession(request.session());
	}
	else {
		reply.SetSession(m_id);
	}
	switch (request.method())
	{
	case 0://OPTIONS
		reply.SetOptins("public: OPTIONS,DESCRIBE,SETUP,PLAY,TEARDOWN\r\n"); 
		break;
	case 1://DESCRIBE
	{
		Buffer sdp;
		sdp << "v=0\r\n";
		sdp << "o=- " <<m_id.c_str() << "1 IN IP4 127.0.0.1\r\n";
		sdp << "t=0 0\r\n" << "a=control:*\r\n" << "m=video 0 RTP/AVP 96\r\n";
		sdp << "a=farmerate:24\r\n";
		sdp << "a=rtpmap:96 H264/90000\r\n" << "a=control:track0\r\n";
		reply.SetSdp(sdp );
	}
		break;
	case 2://SETUP
		reply.SetClientPort(request.port().c_str(),request.port(1).c_str());
		reply.SetServerPort("50000", "55001");
		reply.SetSession(m_id);
		break;
	case 3://PLAY
 	case 4://TEARDOWN
		break;
	}
	return reply;
}

RTSPRquest::RTSPRquest()
{
	m_method = -1;
}

RTSPRquest& RTSPRquest::operator=(const RTSPRquest& protocol)
{
	if (this!=&protocol)
	{
		m_method = protocol.m_method;
		m_session = protocol.m_session;
		m_seq = protocol.m_seq;
		m_url = protocol.m_url;
		m_seq = protocol.m_seq;
		m_client_port[0] = protocol.m_client_port[0];
		m_client_port[1] = protocol.m_client_port[1];
	}
	return *this;
}

RTSPRquest::RTSPRquest(const RTSPRquest& protocol)
{
	m_method = protocol.m_method;
	m_session = protocol.m_session;
	m_seq = protocol.m_seq;
	m_url = protocol.m_url;
	m_seq = protocol.m_seq;
	m_client_port[0] = protocol.m_client_port[0];
	m_client_port[1] = protocol.m_client_port[1];
}

RTSPRquest::~RTSPRquest()
{
	m_method = -1;
}

void RTSPRquest::SetMethod(const Buffer& method)
{
	if (strcmp(method,"OPTIONS")==0) m_method = 0;
	else if (strcmp(method, "DESCRIBE")==0) m_method = 1;
	else if (strcmp(method, "SETUP")==0) m_method = 2;
	else if (strcmp(method, "PLAY")==0) m_method = 3;
	else if (strcmp(method,"TEARDOWN")==0) m_method = 4;
}

void RTSPRquest::SetUrl(const Buffer& url)
{
	m_url = url;
}

void RTSPRquest::SetSequence(const Buffer& seq)
{
	m_seq = seq;
}

void RTSPRquest::SetClientPort(int prots[])
{
	m_client_port[0]<<prots[0];
	m_client_port[1]<<prots[1];
}

void RTSPRquest::SetSession(const Buffer& session)
{
	m_session = session.c_str();
}

RTSPResponse::RTSPResponse()
{
	m_method = -1;
}

RTSPResponse::~RTSPResponse()
{
}

RTSPResponse& RTSPResponse::operator=(const RTSPResponse& protocol)
{
	if (this!=&protocol)
	{
		m_method = protocol.m_method;
		m_options = protocol.m_options;
		m_session = protocol.m_session;
		m_sdp = protocol.m_sdp;
		m_client_port[0] = protocol.m_client_port[0];
		m_client_port[1] = protocol.m_client_port[1];
		m_server_port[0] = protocol.m_server_port[0];
		m_server_port[1] = protocol.m_server_port[1];
	}
	return *this;
}

RTSPResponse::RTSPResponse(const RTSPResponse& protocol)
{
	m_method = protocol.m_method;
	m_options = protocol.m_options;
	m_session = protocol.m_session;
	m_sdp = protocol.m_sdp;
	m_client_port[0] = protocol.m_client_port[0];
	m_client_port[1] = protocol.m_client_port[1];
	m_server_port[0] = protocol.m_server_port[0];
	m_server_port[1] = protocol.m_server_port[1];
}

Buffer RTSPResponse::toBuffer()
{
	Buffer result;
	result += "RTSP/1.0 200 OK\r\n";
	result << "CSeq: " << m_seq.c_str() << "\r\n";
	switch (m_method)
	{
	case 0:
		result<<"public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n\r\n";
		break;
	case 1:
		result << "Content-Base: 127.0.0.1\r\n";
		result << "Content-type: application/sdp\r\n";
		result << "Content-length: " << m_sdp.size() << "\r\n\r\n";
		result << m_sdp.c_str();
		break;
	case 2:
		result << "Transport: RTP/AVP;unicast;client_port="<< m_client_port[0] << "-" << m_client_port[1];
		result << ";server_prot=" << m_server_port[0] << "-" << m_server_port[1] << "\r\n";
		result << "Session: " << m_session.c_str() << "\r\n\r\n";
		break;
	case 3:
		result << "Range: npt=0.000-\r\n";
		result << "Session: " << m_session.c_str() << "\r\n\r\n";
		break;
	case 4:
		result << "Session: " << m_session.c_str() << "\r\n\r\n";
		break;
	}
	return result;
}

void RTSPResponse::SetOptins(const Buffer& options)
{
	m_options = options.c_str();
}

void RTSPResponse::SetSequence(const Buffer& seq)
{
	m_seq = seq.c_str();
}

void RTSPResponse::SetSdp(const Buffer& sdp)
{
	m_sdp = sdp.c_str();
}

void RTSPResponse::SetClientPort( const Buffer& prot0, const Buffer& prot1)
{
	prot0 >> m_client_port[0];
	prot1>>m_client_port[1];
}

void RTSPResponse::SetServerPort(const Buffer&  port0, const Buffer& port1)
{
	port0 >> m_server_port[0];
	port1 >> m_server_port[1];
}

void RTSPResponse::SetSession(const Buffer& session)
{
		m_session = session.c_str();
}

void RTSPResponse::SetMethod(int medthod)
{
	m_method = medthod;
}

EAddr RTSPSessions::GetClientUdpAddr()const
{
	EAddr addr;
	int len = addr.size();
	getsockname(m_client, addr, &len);
	addr.Update();
	addr = m_port;
	return addr;
}
