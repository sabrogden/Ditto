#pragma once
#include "cpprest/http_client.h"

class DropboxRestClient
{
public:
	DropboxRestClient();
	~DropboxRestClient();

	static void Authenticate();
	void UploadFile(std::wstring LocalFiletoUpload);
	void ListFolder();
	void ListFolderContinue();
	
	void LongPoll();

protected:
	//http_client GetApiClient(CString url);

	void ListFolderReturn(web::http::http_response &httpResponse, bool &hasMore);
};

