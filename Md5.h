#pragma once

#define _ReadBufSize 1000000

class CMd5  
{
public:
	char* CalcMD5FromString(const char *s8_Input);
	char* CalcMD5FromFile(const TCHAR *s8_Path);

	void MD5Init();
	void MD5Update(unsigned char *buf, unsigned len);
	char* MD5FinalToString();

	void FreeBuffer();
	CMd5();
	virtual ~CMd5();

private:
	struct MD5Context 
	{
		unsigned long buf[4];
		unsigned long bits[2];
		unsigned char in[64];
	};
		
	void MD5Final (unsigned char digest[16]);
	void MD5Transform(unsigned long buf[4], unsigned long in[16]);
	void byteReverse (unsigned char *buf, unsigned longs);

	char *mp_s8ReadBuffer;
	MD5Context ctx;
	char   ms8_MD5[40]; // Output buffer
};