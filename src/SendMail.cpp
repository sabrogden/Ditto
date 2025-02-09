#include "stdafx.h"
#include "SendMail.h"
#include "Path.h"
#include <mapi.h>
#include "Misc.h"


bool SendMail::Send(CString subject, CString body, CString attachmentFileName)
{
	HINSTANCE hMAPI = ::LoadLibraryA(("MAPI32.DLL"));
	if (!hMAPI)
	{
		log(_T("failed to load MAPI32.DLL"));
		return false;
	}

	// Grab the exported entry point for the MAPISendMail function
	ULONG(PASCAL *SendMail)(ULONG, ULONG_PTR, MapiMessageW*, FLAGS, ULONG);
	(FARPROC&)SendMail = GetProcAddress(hMAPI, "MAPISendMailW");

	if (!SendMail)
	{
		log(_T("failed to load MAPISendMailW from MAPI32.DLL"));
		return false;
	}

	MapiMessageW message;
	::ZeroMemory(&message, sizeof(message));
	message.lpszNoteText = body.GetBuffer();
	message.lpszSubject = subject.GetBuffer();

	CString attachFileName;

	if (attachmentFileName != _T(""))
	{
		MapiFileDescW fileDesc;
		::ZeroMemory(&fileDesc, sizeof(fileDesc));
		fileDesc.nPosition = (ULONG)-1;
		fileDesc.lpszPathName = attachmentFileName.GetBuffer();

		using namespace nsPath;
		CPath path(fileDesc.lpszPathName);
		attachFileName = path.GetName();

		fileDesc.lpszFileName = attachFileName.GetBuffer();

		message.nFileCount = 1;
		message.lpFiles = &fileDesc;
	}

	// Ok to send
	int nError = SendMail(0, (ULONG_PTR)0, &message, MAPI_DIALOG_MODELESS, 0);

	if (nError != SUCCESS_SUCCESS)
	{
		log(StrF(_T("SendMail failed, return %d"), nError));
		return false;
	}

	return true;
}
