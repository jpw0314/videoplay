#include "Tool.h"
#include <Windows.h>
void Trace(const char* format, ...)
{
	{
		va_list ap;
		va_start(ap, format);
		std::string buf;
		buf.resize(1024 * 10);
		vsprintf((char*)buf.c_str(), format, ap);
		OutputDebugStringA((char*)buf.c_str());
		va_end(ap);
	}
}
