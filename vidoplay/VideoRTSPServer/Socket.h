#pragma once
#include <WinSock2.h>
#include <string>
#include <share.h>

class Socket
{
public:
	//0ÊÇUDP 1ÊÇTCP
	Socket(BOOL BisTcp=TRUE){
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
	CSock(bool IsTcp=true):m_sock(new Socket(IsTcp)){}
	~CSock() {
		m_sock.reset();
	}
	CSock(const CSock& sock) :m_sock(sock.m_sock){}
	CSock& operator=(const CSock& sock)
	{
		if (this != &sock)
		{ 
			m_sock = sock.m_sock;
		}
		return *this;
	}
	operator SOCKET() {
		return *m_sock;
	}
private:

	std::shared_ptr<Socket> m_sock;
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

