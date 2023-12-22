#pragma once
#include "Tool.h"
class CMediaFile
{
public:
	CMediaFile();
	~CMediaFile();
	int Open(const Buffer& path,int nType);
	//如果 Buffer的size为0则表示没有帧了
	Buffer ReadOneframe();
	//重置后，ReadOneframe又会有值返回
	void Reset();
	void Close();
private:
	long m_size;
	long FindH264head(int& headsize);
	Buffer ReadH264Frame();
private:
	FILE* m_file;
	//Buffer m_filepath;
	// 96 H264
	int m_type;
};

