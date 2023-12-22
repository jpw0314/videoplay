#pragma once
#include <string>
class Buffer :public std::string
{
public:
	Buffer(): std::string() {}
	Buffer(const std::string& str):std::string(str) {}
	Buffer(const char* str):std::string(str) {}
	Buffer(size_t size) :std::string() { 
		resize(size); 
		memset(*this, 0, this->size());//this Íü¼Ç¼Ó*
	}
	Buffer(const char* str, size_t length):std::string() {
		resize(length);
		memcpy((char*)c_str(), str, length);
	}
	~Buffer()
	{
		std::string::~basic_string();
	}

	void Zero()
	{
		memset((char*)c_str(),0, size());
	}
	operator char* ()const { return (char*)c_str(); }
	operator const char* ()const { return (char*)c_str(); }
	operator unsigned char* () { return (unsigned char*)c_str(); }
	operator const void* () { return c_str(); }
	Buffer& operator<<(const Buffer& buf)
	{
		if (this!=buf)
		{
			*this += buf;

		}
		else
		{
			Buffer temp= buf;
			*this += temp;
		}
		return *this; 
	}
	Buffer& operator<<(int data)
	{
		char s[16] = "";
		snprintf(s, sizeof(s), "%d", data);
		*this += s;
		return *this;
	}

	const Buffer& operator>>(unsigned short& data)const
	{
		data = (unsigned short)atoi(c_str());
		return *this;
	}

};

void Trace(const char* format, ...);
#ifndef TRACE
#define  TRACE Trace
#endif

