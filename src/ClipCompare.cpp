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
			bool saveUtf8 = true;

			if (CGetSetOptions::GetPreferUtf8ForCompare() == FALSE)
			{
				Log(StrF(_T("CClipCompare::Compare, option is set to not use utf8")));
				saveUtf8 = false;
			}
			
			if(leftClip.GetUnicodeTextFormat() == _T("") || rightClip.GetUnicodeTextFormat() == _T(""))
			{
				saveW = false;
				saveUtf8 = false;
			}

			if(leftClip.GetCFTextTextFormat() == "" || rightClip.GetCFTextTextFormat() == "")
			{
				saveA = false;
			}

			if(saveW || saveA || saveUtf8)
			{
				CString leftFile = SaveToFile(leftId, &leftClip, saveW, saveA, saveUtf8);
				CString rightFile = SaveToFile(rightId, &rightClip, saveW, saveA, saveUtf8);

				CString params = _T("");
				CString path = GetComparePath(params);

				if(path != _T(""))
				{
					SHELLEXECUTEINFO sei = { sizeof(sei) };
					sei.lpFile = path;
					CString csParam;
					csParam.Format(_T("%s\"%s\" \"%s\""), params, leftFile, rightFile);
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

CString CClipCompare::GetComparePath(CString &params)
{
	CString path = CGetSetOptions::GetDiffApp().MakeLower();

	if(path != _T(""))
	{
		if (path.Find(_T("totalcmd.exe")) != -1 || 
			path.Find(_T("totalcmd64.exe")) != -1)
		{
			params = _T(" /S=C ");
		}
		else if (path.Find(_T("code.exe")) != -1)
		{
			params = _T(" --diff ");
		}
		return path;
	}

	path = _T("C:\\Program Files\\Beyond Compare 5\\BCompare.exe");
	if (FileExists(path))
	{
		return path;
	}

	path = _T("C:\\Program Files (x86)\\Beyond Compare 4\\BCompare.exe");
	if (FileExists(path))
	{
		return path;
	}

	path = _T("C:\\Program Files\\Beyond Compare 4\\BCompare.exe");
	if (FileExists(path))
	{
		return path;
	}

	path = _T("C:\\Program Files (x86)\\Beyond Compare 3\\BCompare.exe");
	if (FileExists(path))
	{
		return path;
	}

	path = _T("C:\\Program Files\\Beyond Compare 3\\BCompare.exe");
	if (FileExists(path))
	{
		return path;
	}

	path = _T("C:\\Program Files (x86)\\WinMerge\\WinMergeU.exe");
	if(FileExists(path))
	{
		return path;
	}

	path = _T("C:\\Program Files\\WinMerge\\WinMergeU.exe");
	if (FileExists(path))
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

	path = _T("C:\\Program Files (x86)\\Perforce\\p4merge.exe");
	if (FileExists(path))
	{
		return path;
	}

	path = _T("C:\\Program Files\\Perforce\\p4merge.exe");
	if (FileExists(path))
	{
		return path;
	}

	path = _T("c:\\Program Files\\totalcmd\\totalcmd64.exe");
	if (FileExists(path))
	{
		params = _T(" /S=C ");
		return path;
	}

	path = _T("c:\\Program Files (x86)\\totalcmd\\totalcmd.exe");
	if (FileExists(path))
	{
		params = _T(" /S=C ");
		return path;
	}

	path = _T("c:\\totalcmd\\totalcmd64.exe");
	if (FileExists(path))
	{
		params = _T(" /S=C ");
		return path;
	}

	path = _T("c:\\totalcmd\\totalcmd.exe");
	if (FileExists(path))
	{
		params = _T(" /S=C ");
		return path;
	}

	path = CGetSetOptions::ResolvePath(_T("%localappdata%\\Programs\\Microsoft VS Code\\Code.exe"));
	if (FileExists(path))
	{
		params = _T(" --diff ");
		return path;
	}

	path = _T("C:\\Program Files\\Microsoft VS Code\\Code.exe");
	if (FileExists(path))
	{
		params = _T(" --diff ");
		return path;
	}

	path = _T("C:\\Program Files (x86)\\Microsoft VS Code\\Code.exe");
	if (FileExists(path))
	{
		params = _T(" --diff ");
		return path;
	}

	return _T("");
}

CString CClipCompare::SaveToFile(int id, CClip *pClip, bool saveW, bool saveA, bool saveUtf8)
{
	CString path;
	CString pathCompare = CGetSetOptions::GetPath(PATH_CLIP_DIFF);
	CString cs;
	cs.Format(_T("%sditto_compare_%d.txt"), pathCompare, id);

	if(FileExists(cs))
	{
		for(int i = 0; i < 1000; i++)
		{			
			cs.Format(_T("%sditto_compare_%d.txt"), pathCompare, id);
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
		pClip->WriteTextToFile(path, saveW, saveA, FALSE, FALSE, saveUtf8);
	}

	return path;
}