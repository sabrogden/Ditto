#ifndef _CRC32DYNAMIC_H_
#define _CRC32DYNAMIC_H_

class CCrc32Dynamic
{
public:
	CCrc32Dynamic();
	virtual ~CCrc32Dynamic();

	DWORD GenerateCrc32(const LPBYTE lpbArray, DWORD dSize, DWORD &dwCrc32);

protected:
	inline void CalcCrc32(const BYTE byte, DWORD &dwCrc32) const;
	
	void Init();
	void Free();

	DWORD *m_pdwCrc32Table;
};

#endif
