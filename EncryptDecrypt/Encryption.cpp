// Encryption.cpp: implementation of the CEncryption class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Encryption.h"

#include "MemUtil.h"
#include "rijndael.h"
#include "sha2.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEncryption::CEncryption()
{
	m_dwKeyEncRounds = TD_STD_KEYENCROUNDS;

	memset(m_pMasterKey, 0, 32);

	m_random.Initialize();
}

CEncryption::~CEncryption()
{
	mem_erase(m_pMasterKey, 32);

	m_random.Reset();
}

void CEncryption::Release()
{
	delete this;
}

bool CEncryption::Encrypt(const unsigned char* pInput, int nLenInput, const char* szPassword, 
						 unsigned char*& pOutput, int& nLenOutput)
{
	bool			bResult = false;
	TD_TLHEADER		hdr;
	RD_UINT8		uFinalKey[32];
	unsigned long	uFileSize = 0, uAllocated = 0, pos = 0;
	int				nEncryptedPartSize = 0;

	ASSERT(NULL != pInput);		if(NULL == pInput)		return FALSE;
	ASSERT(0	!= nLenInput);	if(0	== nLenInput)	return FALSE;
	ASSERT(NULL != szPassword); if(NULL == szPassword)	return FALSE;

	uFileSize = nLenInput + sizeof(TD_TLHEADER);

	// Allocate enough memory
	uAllocated = uFileSize + 16;
	pOutput = new unsigned char[uAllocated];
	if(NULL != pOutput)
	{
		unsigned long uKeyLen;

		// Build header structure
		hdr.dwSignature1 = TD_TLSIG_1;
		hdr.dwSignature2 = TD_TLSIG_2;
		hdr.dwKeyEncRounds = m_dwKeyEncRounds;

		// Make up the master key hash seed and the encryption IV
		m_random.GetRandomBuffer(hdr.aMasterSeed, 16);
		m_random.GetRandomBuffer((BYTE *)hdr.aEncryptionIV, 16);
		m_random.GetRandomBuffer(hdr.aMasterSeed2, 32);

		// Create MasterKey by hashing szPassword
		uKeyLen = (unsigned long)strlen(szPassword);
		ASSERT(0 != uKeyLen);
		if(0 != uKeyLen)
		{
			sha256_ctx sha32;

			sha256_begin(&sha32);
			sha256_hash((unsigned char *)szPassword, uKeyLen, &sha32);
			sha256_end(m_pMasterKey, &sha32);

			// Generate m_pTransformedMasterKey from m_pMasterKey
			if(TRUE == _TransformMasterKey(hdr.aMasterSeed2))
			{
				// Hash the master password with the generated hash salt
				sha256_begin(&sha32);
				sha256_hash(hdr.aMasterSeed, 16, &sha32);
				sha256_hash(m_pTransformedMasterKey, 32, &sha32);
				sha256_end((unsigned char *)uFinalKey, &sha32);

				// Hash the tasklist contents
				sha256_begin(&sha32);
				sha256_hash((unsigned char *)pInput, nLenInput, &sha32);
				sha256_end((unsigned char *)hdr.aContentsHash, &sha32);

				// Hash the header
				sha256_begin(&sha32);
				sha256_hash((unsigned char *)&hdr + 32, sizeof(TD_TLHEADER) - 32, &sha32);
				sha256_end((unsigned char *)hdr.aHeaderHash, &sha32);
				
				bResult = true;
			}
		}
	}

	if (bResult)
	{
		bResult = false;
		
		// Now we have all to build up the header
		memcpy(pOutput, &hdr, sizeof(TD_TLHEADER));
	
		Rijndael aes;
		// Initialize Rijndael/AES
		if(RIJNDAEL_SUCCESS == aes.init(Rijndael::CBC, Rijndael::Encrypt, uFinalKey,
			Rijndael::Key32Bytes, hdr.aEncryptionIV) )
		{
			nEncryptedPartSize = aes.padEncrypt((RD_UINT8 *)pInput, nLenInput, (RD_UINT8 *)pOutput + sizeof(TD_TLHEADER));

			// Check if all went correct
			ASSERT(0 <= nEncryptedPartSize);
			if(0 <= nEncryptedPartSize)
			{
				bResult = true; // data encrypted successfully
			}

			nLenOutput = sizeof(TD_TLHEADER) + nEncryptedPartSize;
		}
	}

	if (!bResult)
	{
		SAFE_DELETE_ARRAY(pOutput);
	}

	return (bResult);
}




bool CEncryption::Decrypt(const unsigned char* pInput, int nLenInput, const char* szPassword,
						 unsigned char*& pOutput, int& nLenOutput)
{
	bool			bResult = false;
	TD_TLHEADER		hdr;
	RD_UINT8		uFinalKey[32];
	sha256_ctx		sha32;


	ASSERT(NULL != pInput);						if(NULL == pInput)					return FALSE;
	ASSERT(0	!= nLenInput);					if(0	== nLenInput)				return FALSE;
	ASSERT(NULL != szPassword);					if(NULL == szPassword)				return FALSE;
	ASSERT(sizeof(TD_TLHEADER) <= nLenInput);	if(sizeof(TD_TLHEADER) > nLenInput) return FALSE; 

	// Extract header structure from memory file
	memcpy(&hdr, pInput, sizeof(TD_TLHEADER));

	// Hash the header
	sha256_begin(&sha32);
	sha256_hash((unsigned char *)&hdr + 32, sizeof(TD_TLHEADER) - 32, &sha32);
	sha256_end((unsigned char *)uFinalKey, &sha32);

	// Check if hash of header is the same as stored hash
	// to verify integrity of header
	if(0 == memcmp(hdr.aHeaderHash, uFinalKey, 32))
	{
		// Check if we can open this
		if((hdr.dwSignature1 == TD_TLSIG_1) && (hdr.dwSignature2 == TD_TLSIG_2))
		{
			// Allocate enough memory
			pOutput = new unsigned char[nLenInput];
			if(NULL != pOutput)
			{
				memset(pOutput, 0, nLenInput);
				unsigned long uKeyLen = (unsigned long)strlen(szPassword);

				// Create MasterKey by hashing szPassword
				ASSERT(0 != uKeyLen);
				if(0 != uKeyLen)
				{
					sha256_begin(&sha32);
					sha256_hash((unsigned char *)szPassword, uKeyLen, &sha32);
					sha256_end(m_pMasterKey, &sha32);

					m_dwKeyEncRounds = hdr.dwKeyEncRounds;

					// Generate m_pTransformedMasterKey from m_pMasterKey
					if(TRUE == _TransformMasterKey(hdr.aMasterSeed2))
					{
						// Hash the master password with the generated hash salt
						sha256_begin(&sha32);
						sha256_hash(hdr.aMasterSeed, 16, &sha32);
						sha256_hash(m_pTransformedMasterKey, 32, &sha32);
						sha256_end((unsigned char *)uFinalKey, &sha32);

						bResult = true;
					}
				}
			}
		}
	}

	if (bResult)
	{
		bResult = false;

		Rijndael aes;
		// Initialize Rijndael/AES
		if(RIJNDAEL_SUCCESS == aes.init(Rijndael::CBC, Rijndael::Decrypt, uFinalKey,
			Rijndael::Key32Bytes, hdr.aEncryptionIV) )
		{
			nLenOutput = aes.padDecrypt((RD_UINT8 *)pInput + sizeof(TD_TLHEADER), nLenInput - sizeof(TD_TLHEADER), (RD_UINT8 *)pOutput);

			// Check if all went correct
			ASSERT(0 <= nLenOutput);
			if(0 <= nLenOutput)
			{
				// Check contents correct (with high probability)
				sha256_begin(&sha32);
				sha256_hash((unsigned char *)pOutput, nLenOutput, &sha32);
				sha256_end((unsigned char *)uFinalKey, &sha32);
				if(0 == memcmp(hdr.aContentsHash, uFinalKey, 32))
				{
					bResult = true; // data decrypted successfully
				}
			}
		}
	}

	if (!bResult)
	{
		SAFE_DELETE_ARRAY(pOutput);
	}

	return (bResult);
}

void CEncryption::FreeBuffer(unsigned char*& pBuffer)
{
	SAFE_DELETE_ARRAY(pBuffer);
}



/*
  Copyright (c) 2003/2004, Dominik Reichl <dominik.reichl@t-online.de>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Neither the name of ReichlSoft nor the names of its contributors may be
    used to endorse or promote products derived from this software without
    specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/
// Encrypt the master key a few times to make brute-force key-search harder
BOOL CEncryption::_TransformMasterKey(BYTE *pKeySeed)
{
	Rijndael rijndael;
	RD_UINT8 aKey[32];
	RD_UINT8 aTest[16];
	RD_UINT8 aRef[16] = { // The Rijndael class will be tested, that's the expected ciphertext
		0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf,
		0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89
	};
	DWORD i;
	sha256_ctx sha2;

	ASSERT(pKeySeed != NULL); if(pKeySeed == NULL) return FALSE;

	if(rijndael.init(Rijndael::ECB, Rijndael::Encrypt, (const RD_UINT8 *)pKeySeed,
		Rijndael::Key32Bytes, 0) != RIJNDAEL_SUCCESS)
	{
		return FALSE;
	}

	memcpy(m_pTransformedMasterKey, m_pMasterKey, 32);

	for(i = 0; i < m_dwKeyEncRounds; i++)
	{
		rijndael.blockEncrypt((const RD_UINT8 *)m_pTransformedMasterKey, 256, (RD_UINT8 *)m_pTransformedMasterKey);
	}

	// Do a quick test if the Rijndael class worked correctly
	for(i = 0; i < 32; i++) aKey[i] = (RD_UINT8)i;
	for(i = 0; i < 16; i++) aTest[i] = ((RD_UINT8)i << 4) | (RD_UINT8)i;
	if(rijndael.init(Rijndael::ECB, Rijndael::Encrypt, aKey, Rijndael::Key32Bytes, NULL) != RIJNDAEL_SUCCESS)
		{ ASSERT(FALSE); return FALSE; }
	if(rijndael.blockEncrypt(aTest, 128, aTest) != 128) { ASSERT(FALSE); }
	if(memcmp(aTest, aRef, 16) != 0) { ASSERT(FALSE); return FALSE; }

	// Hash once with SHA-256
	sha256_begin(&sha2);
	sha256_hash(m_pTransformedMasterKey, 32, &sha2);
	sha256_end(m_pTransformedMasterKey, &sha2);

	return TRUE;
}


