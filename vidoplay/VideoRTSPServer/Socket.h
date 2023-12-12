#pragma once
#include <WinSock2.h>
#include <string>
#include <share.h>
#include "Tool.h"

class EAddr
{
public:
	EAddr() {
		m_port = -1;
		memset(&m_addr, 0, sizeof(m_addr));
		m_addr.sin_family = PF_INET;
	}

	EAddr(const EAddr& addr)
	{
		m_ip = addr.m_ip;
		m_port = addr.m_port;
		memcpy(&m_addr, &addr.m_addr, sizeof(m_addr));
	}
	EAddr& operator=(const EAddr& addr)
	{
		if (&addr != this)
		{
			m_ip = addr.m_ip;
			m_port = addr.m_port;
			memcpy(&m_addr, &addr.m_addr, sizeof(m_addr));
		}
		return *this;
	}

	~EAddr()
	{

	}
	void Update(const std::string& ip, short port)
	{
		m_port = port;
		m_ip = ip;
		m_addr.sin_port = htons(port);
		m_addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	}
	operator sockaddr_in* ()
	{
		return &m_addr;
	}
	operator const sockaddr* ()const
	{
		return (sockaddr*)&m_addr;
	}
	operator sockaddr* ()
	{
		return (sockaddr*)&m_addr;
	}

	int size()const { return sizeof(sockaddr_in); }
private:
	std::string m_ip;
	short m_port;
	sockaddr_in m_addr;
};

class Socket
{
public:
	//0ÊÇUDP 1ÊÇTCP
	Socket(BOOL BisTcp=true){
		m_sock = INVALID_SOCKET;
		if (BisTcp){
			m_sock = socket(PF_INET, SOCK_STREAM, 0);
		}
		else{
			m_sock = socket(PF_INET, SOCK_STREAM, 0);
		}

	}
	Socket(SOCKET S){
		m_sock = S;
	}

	void Close()
	{
		if (m_sock!=INVALID_SOCKET)
		{
			SOCKET temp = m_sock;
			m_sock = INVALID_SOCKET;
			closesocket(temp);
		}
	}
	~Socket(){
		Close();
	}
	operator SOCKET() {
		return m_sock;
	}

private:
	SOCKET m_sock;
};


class CSock
{
public:
	CSock(bool IsTcp=true):m_isTcp(IsTcp), m_sock(new Socket(IsTcp)) {}
	~CSock() {
		m_sock.reset();
	}
	CSock(SOCKET sock, bool IsTcp):m_sock(new Socket(sock))
	{
		m_isTcp = IsTcp;
	}
	CSock(const CSock& sock) :m_sock(sock.m_sock),m_isTcp(sock.m_isTcp){}
	CSock& operator=(const CSock& sock)
	{
		if (this != &sock)
		{ 
			m_sock = sock.m_sock;
			m_isTcp = sock.m_isTcp;
		}
		return *this;
	}
	operator SOCKET() {
		return *m_sock;
	}
	int Bind(const EAddr& addr)
	{
		if (m_sock==nullptr)
		{
			m_sock.reset(new Socket(m_isTcp));
		}
		return bind(*m_sock, addr, addr.size());
	}
	int Listen(int backlog=5)
	{
		return listen(*m_sock, backlog);
	}
	int Connect(const EAddr& addr)
	{
		return connect(*m_sock, addr, addr.size());
	}
	CSock Accept(EAddr& addr)
	{
		int len = addr.size();
		SOCKET S= accept(*m_sock, addr, &len);
		return CSock(S, m_isTcp);
	}
	 
	int Recv(const Buffer& data)
	{
		return recv(*m_sock, data, data.size(), 0);
	}
	int Send(const Buffer& data)
	{
		send(*m_sock, data, data.size(),0);
	}

	void Close()
	{

		m_sock.reset();

	}
private:
	std::shared_ptr<Socket> m_sock;
	bool m_isTcp;
};

class socketinit {
public:
	socketinit() {
		WSAData data;
		WSAStartup(MAKEWORD(2,2), &data);
	}
	~socketinit()
	{
		WSACleanup();
	}
};

