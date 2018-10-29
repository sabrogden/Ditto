#include "stdafx.h"
#include "DropboxRestClient.h"

#include "cpprest/http_listener.h"
#include "cpprest/http_client.h"
#include "cpprest\filestream.h"

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace web::http::oauth2::experimental;
using namespace web::http::experimental::listener;
using namespace concurrency;
using namespace concurrency::streams;

http_client GetApiClient(CString url)
{
	oauth2_config oauth2_config(U("5bb6u8qvhs21whl"), U("h0yeouikmdrmea5"), U("https://www.dropbox.com/oauth2/authorize"), U("https://api.dropboxapi.com/oauth2/token"), U(""));

	oauth2_token token;
	token.set_access_token(U("vsko34wW-9AAAAAAAAAB1GTack-f3GsKqtQIkg0q0wN1SIETTpm6n6Jv5X41WbBj"));
	token.set_token_type(U("bearer"));

	oauth2_config.set_token(token);

	http_client_config http_config;
	http_config.set_oauth2(oauth2_config);

	http_client api(url.GetBuffer(), http_config);
	url.ReleaseBuffer();

	return api;
}


DropboxRestClient::DropboxRestClient()
{
}


DropboxRestClient::~DropboxRestClient()
{
}


void DropboxRestClient::Authenticate()
{
	oauth2_config config(U("5bb6u8qvhs21whl"),
		U("h0yeouikmdrmea5"),
		U("https://www.dropbox.com/oauth2/authorize"),
		U("https://api.dropboxapi.com/oauth2/token"),
		U("https://ditto-cp.sourceforge.io/dropboxlanding.php"));

	auto auth_uri(config.build_authorization_uri(true));
	ShellExecuteA(NULL, "open", conversions::utf16_to_utf8(auth_uri).c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void DropboxRestClient::UploadFile(std::wstring LocalFiletoUpload)
{
	//std::wstring LocalFiletoUpload = U("c:\\AVScanner.ini");

	concurrency::streams::file_stream<unsigned char>::open_istream(LocalFiletoUpload)
		.then([](pplx::task<basic_istream<unsigned char>> previousTask)
	{
		auto fileStream = previousTask.get();
		//get the content length, used to set the Content-Length property
		fileStream.seek(0, std::ios::end);
		auto length = static_cast<size_t>(fileStream.tell());
		fileStream.seek(0, 0);

		web::json::value json;
		json[L"path"] = web::json::value::string(U("/Ditto/MyName/test.txt"));
		json[L"mode"] = web::json::value::string(U("add"));
		json[L"autorename"] = web::json::value::boolean(true);
		json[L"mute"] = web::json::value::boolean(true);
		auto stringValue = json.serialize();

		oauth2_config oauth2_config(U("5bb6u8qvhs21whl"), /* Your live App key*/
			U("h0yeouikmdrmea5"), /* Your live App secret*/
			U("https://www.dropbox.com/oauth2/authorize"), /* Authorization endpoint*/
			U("https://api.dropboxapi.com/oauth2/token"), /* Token endpoint */
			U("")); /* Redirect URI */

		oauth2_token token;
		token.set_access_token(U("vsko34wW-9AAAAAAAAAB1GTack-f3GsKqtQIkg0q0wN1SIETTpm6n6Jv5X41WbBj"));
		token.set_token_type(U("bearer"));

		oauth2_config.set_token(token);

		http_client_config http_config;
		http_config.set_oauth2(oauth2_config);

		http_client api(U("https://content.dropboxapi.com/2/files/upload"), http_config);

		//auto ss = api.request(methods::POST, U("files/upload"));

		http_request request(methods::POST);
		request.headers().add(U("Dropbox-API-Arg"), stringValue);
		request.set_body(fileStream, length);

		api.request(request)
			.then([fileStream](pplx::task<http_response> previousTask)
		{
			fileStream.close();
			std::wostringstream ss;
			try
			{
				auto response = previousTask.get();
				auto body = response.body();
				// log response success code
				ss << L"Server returned status code "
					<< response.status_code() << L"."
					<< std::endl;
				OutputDebugString(ss.str().data());
			}
			catch (const http_exception& e)
			{
				ss << e.what() << std::endl;
				OutputDebugString(ss.str().data());
			}
		});
	});
}

void DropboxRestClient::DownloadFile(CString file2)
{
	auto fileStream = std::make_shared<ostream>();

	utility::string_t s = file2.GetBuffer();

	// Open stream to output file.
	pplx::task<void> requestTask = fstream::open_ostream(U("results.html")).then([=](ostream outFile)
	{
		*fileStream = outFile;

		web::json::value json;
		json[L"path"] = web::json::value::string(s);
		auto stringValue = json.serialize();
		http_client api = GetApiClient(U("https://content.dropboxapi.com/2/files/download"));
		http_request request(methods::POST);
		request.headers().add(U("Dropbox-API-Arg"), stringValue);

		return api.request(request);
	})
		// Handle response headers arriving.
	.then([=](http_response response)
	{
		printf("Received response status code:%u\n", response.status_code());

		// Write response body into the file.
		return response.body().read_to_end(fileStream->streambuf());
	})

		// Close the file stream.
	.then([=](size_t)
	{
		return fileStream->close();
	});

	try
	{
		requestTask.wait();
	}
	catch (const std::exception &e)
	{
		printf("Error exception:%s\n", e.what());
	}
}

CString lastCursor;
void DropboxRestClient::ListFolder()
{
	web::json::value json;
	json[L"path"] = web::json::value::string(U("/Ditto/sabrogden@gmail.com-home"));
	json[L"recursive"] = web::json::value::boolean(true);
	json[L"include_media_info"] = web::json::value::boolean(false);
	json[L"include_deleted"] = web::json::value::boolean(false);
	json[L"include_has_explicit_shared_members"] = web::json::value::boolean(false);
	auto stringValue = json.serialize();

	http_client api = GetApiClient(U("https://api.dropboxapi.com/2/files/list_folder"));

	http_request request(methods::POST);
	request.headers().add(U("Content-Type"), U("application/json"));
	request.set_body(stringValue);

	api.request(request).then([](pplx::task<http_response> previousTask)
	{
		try
		{
			auto response = previousTask.get();
			if (response.status_code() == status_codes::OK)
			{
				bool hasMore = false;
				ListFolderReturn(response, hasMore);
				if (hasMore)
				{
					ListFolderContinue();
				}
			}
		}
		catch (const http_exception& e)
		{
			//OutputDebugString(ss.str().data());
		}

		LongPoll();
	});
}

void DropboxRestClient::ListFolderContinue()
{
	web::json::value json;
	json[L"cursor"] = web::json::value::string(lastCursor.GetBuffer());
	auto stringValue = json.serialize();

	http_client api = GetApiClient(U("https://api.dropboxapi.com/2/files/list_folder/continue"));

	http_request request(methods::POST);
	request.headers().add(U("Content-Type"), U("application/json"));
	request.set_body(stringValue);

	auto response = api.request(request).get();
	try
	{
		if (response.status_code() == status_codes::OK)
		{
			bool hasMore = false;
			ListFolderReturn(response, hasMore);
			if (hasMore)
			{
				ListFolderContinue();
			}
		}
	}
	catch (const http_exception& e)
	{
		//OutputDebugString(ss.str().data());
	}
}

void DropboxRestClient::ListFolderReturn(web::http::http_response &httpResponse, bool &hasMore)
{
	auto jsonData = httpResponse.extract_json().get();

	std::wstring s = jsonData[L"cursor"].as_string();
	hasMore = jsonData[L"has_more"].as_bool();

	lastCursor = s.c_str();

	auto fileEntries = jsonData.at(U("entries"));

	for (auto const & e : fileEntries.as_array())
	{
		if (e.at(L".tag").as_string() == L"file")
		{
			CString file = e.at(L"path_lower").as_string().c_str();

			DownloadFile(file);

			COleDateTime dt;

			auto dataString = jsonData.serialize();
			auto dataString2 = e.serialize();

			CString value = e.at(L"server_modified").as_string().c_str();
			dt.ParseDateTime(value);
			int k = 0;
		}
	}
}

void DropboxRestClient::LongPoll()
{
	try
	{
		web::json::value json;
		json[L"cursor"] = web::json::value::string(lastCursor.GetBuffer());
		lastCursor.ReleaseBuffer();
		json[L"timeout"] = web::json::value::number(120);

		auto stringValue = json.serialize();

		http_client api(U("https://notify.dropboxapi.com/2/files/list_folder/longpoll"));

		http_request request(methods::POST);
		request.headers().add(U("Content-Type"), U("application/json"));
		request.set_body(stringValue);

		api.request(request)
			.then([](pplx::task<http_response> previousTask)
		{
			try
			{
				auto response = previousTask.get();
				if (response.status_code() == status_codes::OK)
				{
					auto jsonData = response.extract_json().get();

					auto changes = jsonData[L"changes"].as_bool();
					if (changes)
					{
						ListFolderContinue();
					}
				}
			}
			catch (const task_canceled& e)
			{
				int x = 0;
			}
			catch (const http_exception& e)
			{
				int y = 0;
			}			

			LongPoll();
		});
	}
	catch (const std::exception& e)
	{
		int x = 0;
	}
}

