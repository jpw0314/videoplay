#pragma once
#include <string>
#include <string.h>
class Buffer :public std::string
{
public:
	Buffer() :std::string() {}
	Buffer(const std::string& str) :std::string(str) {}
	Buffer(const char* str) :std::string(str) {}
	Buffer(size_t size) :std::string() { resize(size); }
	Buffer(const char* str, size_t length) :std::string() {
		resize(length);
		memcpy((char*)c_str(), str, length);
	}
	operator char* ()const { return (char*)c_str(); }
	operator const char* ()const { return (char*)c_str(); }
	operator unsigned char* () { return (unsigned char*)c_str(); }
	operator const void* () { return c_str(); }
};