#pragma once
#include "Tool.h"
class CMediaFile
{
public:
	CMediaFile();
	~CMediaFile();
	int Open(const Buffer& path,int nType);
	//��� Buffer��sizeΪ0���ʾû��֡��
	Buffer ReadOneframe();
	//���ú�ReadOneframe�ֻ���ֵ����
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

