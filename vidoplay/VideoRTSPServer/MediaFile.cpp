#include "MediaFile.h"

CMediaFile::CMediaFile()
{
	m_file = NULL;
	m_type = -1;
}

CMediaFile::~CMediaFile()
{
	Close();
}

int CMediaFile::Open(const Buffer& path, int nType)
{
	m_file = fopen(path, "rb");
	 if (m_file==NULL) return -1;
	 m_type = nType;
	 fseek(m_file, 0, SEEK_END);
	 m_size = ftell(m_file);
	 Reset();
	return 0;
}

Buffer CMediaFile::ReadOneframe()
{
	switch (m_type)
	{
	case 96:
		return ReadH264Frame();
	}
	return Buffer();
}

void CMediaFile::Reset()
{
	if (m_file)
	{
		fseek(m_file, 0, SEEK_SET);
	}
}

void CMediaFile::Close()
{
	m_type = -1;
	if (m_file != NULL)
	{
		FILE* temp = m_file;
		m_file = NULL;
		fclose(m_file);
	}
}

long CMediaFile::FindH264head(int& headsize)
{
	while (!feof(m_file))
	{
		char c = 0x7f;
		while (!feof(m_file))// file end of file
		{
			c = fgetc(m_file);
			if (c == 0) break;
		}
		if (!feof(m_file))
		{
			c = fgetc(m_file);
			if (c == 0)
			{
				c = fgetc(m_file);
				if (c == 1)
				{
					headsize = 3;
					return ftell(m_file)-3;
				}
				else if (c == 0)
				{
					c = fgetc(m_file);
					if (c == 1)
					{
						headsize = 4;
						return (ftell(m_file)-4);
					}
				}
			}
		}
	}
	return -1;
}

Buffer CMediaFile::ReadH264Frame()
{
	if (m_file)
	{
		int headsize = 0;
		long ret = FindH264head(headsize);
		if (ret==-1)return Buffer();
		fseek(m_file, ret+headsize, SEEK_SET);
		long tail = FindH264head(headsize);
		if (tail == -1) tail = m_size;
		long size = tail - ret;
		fseek(m_file, ret, SEEK_SET);
		Buffer result(size);
		fread(result,1,size, m_file);
		return result;
	}
	return Buffer();
}
