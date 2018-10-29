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
	static void ListFolderContinue();
	static void DownloadFile(CString file);
	static void LongPoll();

protected:
	//http_client GetApiClient(CString url);

	static void ListFolderReturn(web::http::http_response &httpResponse, bool &hasMore);
};

