#include "stdafx.h"
#include "ClipCompare.h"
#include "Misc.h"
#include "Options.h"

CClipCompare::CClipCompare(void)
{
}


CClipCompare::~CClipCompare(void)
{
}


void CClipCompare::Compare(int leftId, int rightId)
{
	CClip leftClip;
	if(leftClip.LoadFormats(leftId, true))
	{
		CClip rightClip;
		if(rightClip.LoadFormats(rightId, true))
		{
			bool saveW = true;
			bool saveA = true;
			
			if(leftClip.GetUnicodeTextFormat() == _T("") || rightClip.GetUnicodeTextFormat() == _T(""))
			{
				saveW = false;
			}

			if(leftClip.GetCFTextTextFormat() == "" || rightClip.GetCFTextTextFormat() == "")
			{
				saveA = false;
			}

			if(saveW || saveA)
			{
				CString leftFile = SaveToFile(leftId, &leftClip, saveW, saveA);
				CString rightFile = SaveToFile(rightId, &rightClip, saveW, saveA);

				CString path = GetComparePath();

				if(path != _T(""))
				{
					SHELLEXECUTEINFO sei = { sizeof(sei) };
					sei.lpFile = path;
					CString csParam;
					csParam.Format(_T("%s %s"), leftFile, rightFile);
					sei.lpParameters = csParam;
					sei.nShow = SW_NORMAL;

					Log(StrF(_T("Comparing two clips, left Id %d, right Id %d, Path: %s %s"), leftId, rightId, path, csParam));

					if (!ShellExecuteEx(&sei))
					{
					}
				}
				else
				{
					Log(StrF(_T("CClipCompare::Compare, No Valid compare apps, not doing compare")));

					ShellExecute(NULL, _T("open"), _T("http://winmerge.org/"), NULL,NULL, SW_SHOW);
				}
			}
			else	
			{
				Log(StrF(_T("CClipCompare::Compare, did not find valid text for both passed in clips")));
			}
		}
		else
		{
			Log(StrF(_T("CClipCompare::Compare, Failed to load RIGHT clip formats Id: %d"), rightId));
		}
	}
	else
	{
		Log(StrF(_T("CClipCompare::Compare, Failed to load LEFT clip formats Id: %d"), leftId));
	}
}

CString CClipCompare::GetComparePath()
{
	CString path = CGetSetOptions::GetDiffApp();

	if(path != _T(""))
	{
		return path;
	}

	path = _T("C:\\Program Files (x86)\\Beyond Compare 3\\BCompare.exe");
	if(FileExists(path))
	{
		return path;
	}

	path = _T("C:\\Program Files\\Beyond Compare 3\\BCompare.exe");
	if(FileExists(path))
	{
		return path;
	}

	path = _T("C:\\Program Files (x86)\\WinMerge\\WinMergeU.exe");
	if(FileExists(path))
	{
		return path;
	}

	path = _T("C:\\Program Files (x86)\\Araxis\\Araxis Merge\\compare.exe");
	if(FileExists(path))
	{
		return path;
	}

	path = _T("C:\\Program Files\\Araxis\\Araxis Merge\\compare.exe");
	if(FileExists(path))
	{
		return path;
	}

	return _T("");
}

CString CClipCompare::SaveToFile(int id, CClip *pClip, bool saveW, bool SaveA)
{
	CString path;
	wchar_t wchPath[MAX_PATH];
	if (GetTempPathW(MAX_PATH, wchPath))
	{
		CString cs;
		cs.Format(_T("%sditto_compare_%d.txt"), wchPath, id);

		if(FileExists(cs))
		{
			for(int i = 0; i < 1000; i++)
			{			
				cs.Format(_T("%sditto_compare_%d.txt"), wchPath, id);
				if(FileExists(cs))
				{
					path = cs;
					break;
				}
			}
		}
		else
		{
			path = cs;
		}

		if(path != _T("") && 
			pClip != NULL)
		{
			pClip->WriteTextToFile(path, saveW, SaveA, false);
		}
	}

	return path;
}