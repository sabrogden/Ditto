#pragma once
class CCreateQRCodeImage
{
public:
	CCreateQRCodeImage(void);
	~CCreateQRCodeImage(void);

	unsigned char* CreateImage(CString text, int &size);
};

