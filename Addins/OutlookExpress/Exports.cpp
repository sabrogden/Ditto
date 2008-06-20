#include "StdAfx.h"
#include ".\exports.h"
#include "TextConvert.h"

bool WriteDataToFile(CString csPath, LPVOID data, ULONG size);
CString g_csDIBImagePath = _T("");
int g_nDIBImageName = 1;

bool DittoAddin(const CDittoInfo &DittoInfo, CDittoAddinInfo &info)
{
	if(DittoInfo.ValidateSize() == false || info.ValidateSize() == false)
	{
		CString csError;
		csError.Format(_T("OutLookExpress Addin - Passed in structures are of different size, DittoInfo Passed: %d, Local: %d, DittoAddinInfo Passed: %d, Local: %d"), DittoInfo.m_nSizeOfThis, sizeof(CDittoInfo), info.m_nSizeOfThis, sizeof(CDittoAddinInfo));
		OutputDebugString(csError);
		return false;
	}

	info.m_Name = _T("Outlook Express");
	info.m_AddinVersion = 1;

	return true;
}

bool SupportedFunctions(const CDittoInfo &DittoInfo, FunctionType type, std::vector<CFunction> &Functions)
{
	CFunction func;
	func.m_csFunction = _T("ConvertPathToHtmlImageTag");
	func.m_csDisplayName = _T("Past As html image link");
	func.m_csDetailDescription = _T("Converts a CF_DIB or CF_HDROP to a html format for pasting into outlook express");

	Functions.push_back(func);

	//CFunction func2;
	//func2.m_csFunction = "ConvertPathToHtmlImageTag";
	//func2.m_csDisplayName = _T("Past As html image link222222222");
	//func2.m_csDetailDescription = _T("Converts a CF_DIB or CF_HDROP to a html format for pasting into outlook express");

	//Functions.push_back(func2);

	return true;
}

void CreateLocalPath(bool bCreateDir)
{
	g_csDIBImagePath = _wgetenv(_T("TMP"));;
	g_csDIBImagePath += _T("\\ditto");
	if(bCreateDir)
	{
		CreateDirectory(g_csDIBImagePath, NULL);
	}
}

bool ConvertPathToHtmlImageTag(const CDittoInfo &DittoInfo, IClip *pClip)
{
	bool bRet = false;
	IClipFormats *pFormats = pClip->Clips();
	if(pFormats)
	{
		if(g_csDIBImagePath.IsEmpty())
		{
			CreateLocalPath(true);
		}

		CString csIMG = _T("");

		IClipFormat *pCF_DIB = pFormats->FindFormatEx(CF_DIB);
		if(pCF_DIB != NULL)
		{
			CString csFile;
			csFile.Format(_T("%s\\%d.bmp"), g_csDIBImagePath, g_nDIBImageName);
			g_nDIBImageName++;


			LPVOID pvData = GlobalLock(pCF_DIB->Data());
			ULONG size = (ULONG)GlobalSize(pCF_DIB->Data());

			if(WriteDataToFile(csFile, pvData, size))
			{
				GlobalUnlock(pCF_DIB->Data());

				csIMG.Format(_T("<IMG src=\"file:///%s\">"), csFile);
			}
			else
			{
				GlobalUnlock(pCF_DIB->Data());
			}
		}
		else
		{
			IClipFormat *pHDrop = pFormats->FindFormatEx(CF_HDROP);
			if(pHDrop)
			{
				HDROP drop = (HDROP)GlobalLock((HDROP)pHDrop->Data());
				int nNumFiles = DragQueryFile(drop, -1, NULL, 0);
				TCHAR file[MAX_PATH];

				for(int nFile = 0; nFile < nNumFiles; nFile++)
				{
					if(DragQueryFile(drop, nFile, file, sizeof(file)) > 0)
					{
						CString csOrigfile(file);
						CString csFile(file);
						csFile = csFile.MakeLower();

						if(csFile.Find(_T(".bmp")) != -1 || 
							csFile.Find(_T(".dib")) != -1 ||
							csFile.Find(_T(".jpg")) != -1 ||
							csFile.Find(_T(".jpeg")) != -1 ||
							csFile.Find(_T(".jpe")) != -1 ||
							csFile.Find(_T(".jfif")) != -1 ||
							csFile.Find(_T(".gif")) != -1 ||
							csFile.Find(_T(".tif")) != -1 ||
							csFile.Find(_T(".tiff")) != -1 ||
							csFile.Find(_T(".png")) != -1)
						{
							CString csFormat;
							csFormat.Format(_T("<IMG src=\"file:///%s\">"), csOrigfile);
							if(nFile < nNumFiles-1)
							{
								csFormat += _T("<br>");
							}
							csIMG += csFormat;
						}
					}
				}

				GlobalUnlock(pHDrop->Data());
			}
		}

		if(csIMG.IsEmpty() == FALSE)
		{
			pFormats->DeleteAll();
			CStringA utf8;
			CTextConvert::ConvertToUTF8(csIMG, utf8);
			pFormats->AddNew(DittoAddinHelpers::GetFormatID(_T("HTML Format")), DittoAddinHelpers::NewGlobalP(utf8.GetBuffer(), utf8.GetLength()));
			bRet = true;
		}
	}

	return bRet;
}

bool WriteDataToFile(CString csPath, LPVOID data, ULONG size)
{
	bool bRet = false;
	CFile file;
	CFileException ex;
	if(file.Open(csPath, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary, &ex))
	{
		BITMAPINFO *lpBI = (BITMAPINFO *)data;

		int nPaletteEntries = 1 << lpBI->bmiHeader.biBitCount;
		if(lpBI->bmiHeader.biBitCount > 8)
			nPaletteEntries = 0;
		else if( lpBI->bmiHeader.biClrUsed != 0 )
			nPaletteEntries = lpBI->bmiHeader.biClrUsed;

		BITMAPFILEHEADER BFH;
		memset(&BFH, 0, sizeof( BITMAPFILEHEADER));
		BFH.bfType = 'MB';
		BFH.bfSize = sizeof(BITMAPFILEHEADER) + size;
		BFH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nPaletteEntries * sizeof(RGBQUAD);

		file.Write(&BFH, sizeof(BITMAPFILEHEADER));
		file.Write(data, size);

		file.Close();

		bRet = true;
	}
	else
	{
		CString csError;
		TCHAR exError[250];
		ex.GetErrorMessage(exError, sizeof(exError));

		csError.Format(_T("OutLookExpress Addin - Failed to write CF_DIB to file: %s, Error: %s"), csPath, exError);
		OutputDebugString(csPath);
	}

	return bRet;
}

bool CleanupPastedImages()
{
	bool bRet = false;
	if(g_csDIBImagePath.IsEmpty())
	{
		CreateLocalPath(false);
	}

	CFileFind find;
	BOOL bCont = find.FindFile(g_csDIBImagePath + _T("\\*"));

	while(bCont)
	{
		bCont = find.FindNextFile();
		DeleteFile(find.GetFilePath());
	}
	find.Close();

	bRet = RemoveDirectory(g_csDIBImagePath) == TRUE;

	return false;;
}
