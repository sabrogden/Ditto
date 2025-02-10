#pragma once
class SendMail
{
public:
	static bool Send(CString subject, CString body, CString attachmentFileName);
};

