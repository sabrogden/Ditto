// OcrThread.cpp: implementation of the COcrThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OcrThread.h"
#include "CP_Main.h"
#include "Clip.h"
#include "Misc.h"
#include <vector>
// cppwinrt still pulls in <experimental/coroutine> when the project
// builds as c++17, newer msvc errors on that header unless this is set,
// this file only, nothing else sees it
#define _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS
// mfc defines TRY as a macro ({ try {), it breaks the winrt headers
// (windows.globalization has a TRY currency code method), drop it for
// the winrt includes only, this file does not use mfc TRY/CATCH
#pragma push_macro("TRY")
#undef TRY
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Storage.Streams.h>
#pragma pop_macro("TRY")

// longest edge of the image handed to the ocr engine, bigger images
// are downscaled first so recognition stays fast
#define OCR_MAX_EDGE 2000
// images smaller than this on the long edge hold no readable text,
// they are icons and the like, skip them for good
#define OCR_MIN_EDGE 64
// most text kept per clip, keeps the database from growing on huge images
#define OCR_MAX_TEXT 8000
// one clip is looked at per tick, so a backlog drains slowly and quietly
#define OCR_TICK_MS 5000
// ticks to wait between looks when the last look found no work,
// 12 ticks is about a minute, keeps an idle database cheap
#define OCR_IDLE_TICKS 12

enum OcrStatus
{
	OCR_STATUS_OK,
	OCR_STATUS_NO_ENGINE,
	OCR_STATUS_FAILED
};

// winrt needs an apartment on this thread, init and uninit around each
// ocr call so no thread lifetime hook is needed
class CApartmentInit
{
public:
	CApartmentInit()
	{
		m_init = false;
		try
		{
			winrt::init_apartment(winrt::apartment_type::multi_threaded);
			m_init = true;
		}
		catch (...)
		{
		}
	}

	~CApartmentInit()
	{
		if (m_init)
		{
			winrt::uninit_apartment();
		}
	}

	bool IsInit() { return m_init; }

private:
	bool m_init;
};

bool GetPngEncoderClsid(CLSID* clsid)
{
	UINT num = 0;
	UINT size = 0;
	if (Gdiplus::GetImageEncodersSize(&num, &size) != Gdiplus::Ok ||
		size == 0)
	{
		return false;
	}

	Gdiplus::ImageCodecInfo* info = (Gdiplus::ImageCodecInfo*)malloc(size);
	if (info == NULL)
	{
		return false;
	}

	bool found = false;
	if (Gdiplus::GetImageEncoders(num, size, info) == Gdiplus::Ok)
	{
		for (UINT i = 0; i < num; i++)
		{
			if (wcscmp(info[i].MimeType, L"image/png") == 0)
			{
				*clsid = info[i].Clsid;
				found = true;
				break;
			}
		}
	}

	free(info);
	return found;
}

bool BitmapToPng(Gdiplus::Bitmap* bitmap, std::vector<BYTE>& png)
{
	CLSID pngClsid;
	if (GetPngEncoderClsid(&pngClsid) == false)
	{
		return false;
	}

	IStream* stream = NULL;
	if (::CreateStreamOnHGlobal(NULL, TRUE, &stream) != S_OK ||
		stream == NULL)
	{
		return false;
	}

	bool ret = false;
	if (bitmap->Save(stream, &pngClsid, NULL) == Gdiplus::Ok)
	{
		STATSTG stat = { 0 };
		if (stream->Stat(&stat, STATFLAG_NONAME) == S_OK &&
			stat.cbSize.HighPart == 0 &&
			stat.cbSize.LowPart > 0)
		{
			LARGE_INTEGER zero = { 0 };
			if (stream->Seek(zero, STREAM_SEEK_SET, NULL) == S_OK)
			{
				png.resize(stat.cbSize.LowPart);
				ULONG read = 0;
				if (stream->Read(png.data(), (ULONG)png.size(), &read) == S_OK &&
					read == png.size())
				{
					ret = true;
				}
			}
		}
	}

	stream->Release();
	return ret;
}

// returns a new downscaled bitmap, or NULL when no scaling is needed
Gdiplus::Bitmap* DownscaleBitmap(Gdiplus::Bitmap* bitmap, UINT maxEdge)
{
	UINT width = bitmap->GetWidth();
	UINT height = bitmap->GetHeight();
	UINT longEdge = max(width, height);
	if (longEdge <= maxEdge)
	{
		return NULL;
	}

	UINT newWidth = (width * maxEdge) / longEdge;
	UINT newHeight = (height * maxEdge) / longEdge;
	if (newWidth == 0)
	{
		newWidth = 1;
	}
	if (newHeight == 0)
	{
		newHeight = 1;
	}

	Gdiplus::Bitmap* scaled = new Gdiplus::Bitmap(newWidth, newHeight, Gdiplus::PixelFormat32bppARGB);
	Gdiplus::Graphics graphics(scaled);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	graphics.DrawImage(bitmap, 0, 0, (INT)newWidth, (INT)newHeight);

	if (scaled->GetLastStatus() != Gdiplus::Ok)
	{
		delete scaled;
		return NULL;
	}

	return scaled;
}

// reads the text out of one image with the ocr engine built into windows,
// no download, no network, everything stays on this machine
OcrStatus RecognizeBitmapText(Gdiplus::Bitmap* bitmap, CString& ocrText)
{
	ocrText = _T("");

	CApartmentInit apartment;
	if (apartment.IsInit() == false)
	{
		return OCR_STATUS_NO_ENGINE;
	}

	try
	{
		winrt::Windows::Media::Ocr::OcrEngine engine = winrt::Windows::Media::Ocr::OcrEngine::TryCreateFromUserProfileLanguages();
		if (engine == nullptr)
		{
			// no ocr language pack installed, or the api is not
			// available to this process, nothing to do
			return OCR_STATUS_NO_ENGINE;
		}

		std::vector<BYTE> png;
		if (BitmapToPng(bitmap, png) == false)
		{
			return OCR_STATUS_FAILED;
		}

		winrt::Windows::Storage::Streams::InMemoryRandomAccessStream stream;
		winrt::Windows::Storage::Streams::DataWriter writer(stream);
		writer.WriteBytes(winrt::array_view<const uint8_t>(png.data(), png.data() + png.size()));
		writer.StoreAsync().get();
		writer.FlushAsync().get();
		writer.DetachStream();
		stream.Seek(0);

		winrt::Windows::Graphics::Imaging::BitmapDecoder decoder = winrt::Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(stream).get();
		winrt::Windows::Graphics::Imaging::SoftwareBitmap softwareBitmap = decoder.GetSoftwareBitmapAsync(
			winrt::Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8,
			winrt::Windows::Graphics::Imaging::BitmapAlphaMode::Premultiplied).get();

		winrt::Windows::Media::Ocr::OcrResult result = engine.RecognizeAsync(softwareBitmap).get();
		if (result != nullptr)
		{
			ocrText = result.Text().c_str();
		}

		return OCR_STATUS_OK;
	}
	catch (winrt::hresult_error& e)
	{
		Log(StrF(_T("ocr failed, hresult: 0x%x"), e.code().value));
		return OCR_STATUS_FAILED;
	}
	catch (...)
	{
		return OCR_STATUS_FAILED;
	}
}

COcrThread::COcrThread()
{
	m_threadName = "COcrThread";
	m_waitTimeout = OCR_TICK_MS;
	m_engineUnavailable = false;
	m_idleTicks = 0;
}

COcrThread::~COcrThread()
{
}

void COcrThread::StartOcr()
{
	if (CGetSetOptions::GetOcrImageSearch() == FALSE)
	{
		return;
	}

	if (IsRunning())
	{
		return;
	}

	m_engineUnavailable = false;
	m_idleTicks = 0;

	Start();

	if (m_thread != NULL)
	{
		SetThreadPriority(m_thread, THREAD_PRIORITY_LOWEST);
	}
}

void COcrThread::StopOcr()
{
	Stop();
}

void COcrThread::OnTimeOut(void* param)
{
	if (CGetSetOptions::GetOcrImageSearch() == FALSE)
	{
		return;
	}

	if (m_engineUnavailable)
	{
		return;
	}

	if (m_idleTicks > 0)
	{
		m_idleTicks--;
		return;
	}

	int clipId = GetNextClipId();
	if (clipId <= 0)
	{
		// nothing to do, wait a while before looking again
		m_idleTicks = OCR_IDLE_TICKS;
		return;
	}

	ProcessClip(clipId);
}

// newest image clip first that has not been looked at yet, ocrText
// stays NULL until a clip has been through ocr, empty means it was
// checked and holds no text
int COcrThread::GetNextClipId()
{
	int clipId = -1;

	try
	{
		CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT Main.lID FROM Main ")
			_T("INNER JOIN Data ON Data.lParentID = Main.lID ")
			_T("WHERE Main.ocrText IS NULL ")
			_T("AND Data.strClipBoardFormat IN ('CF_DIB', 'PNG') ")
			_T("ORDER BY Main.lID DESC LIMIT 1"));

		if (q.eof() == false)
		{
			clipId = q.getIntField(_T("lID"));
		}
	}
	CATCH_SQLITE_EXCEPTION

	return clipId;
}

bool COcrThread::ProcessClip(int clipId)
{
	CClipFormat format(CF_DIB);
	if (theApp.GetClipData(clipId, format) == FALSE)
	{
		format.m_cfType = theApp.m_PNG_Format;
		if (theApp.GetClipData(clipId, format) == FALSE)
		{
			SaveOcrText(clipId, _T(""));
			return false;
		}
	}

	Gdiplus::Bitmap* bitmap = format.CreateGdiplusBitmap();
	if (bitmap == NULL)
	{
		SaveOcrText(clipId, _T(""));
		return false;
	}

	UINT width = bitmap->GetWidth();
	UINT height = bitmap->GetHeight();
	UINT longEdge = max(width, height);

	if (longEdge < OCR_MIN_EDGE)
	{
		// too small to hold readable text, skip it for good
		delete bitmap;
		SaveOcrText(clipId, _T(""));
		return true;
	}

	Gdiplus::Bitmap* scaled = DownscaleBitmap(bitmap, OCR_MAX_EDGE);
	Gdiplus::Bitmap* toOcr = scaled != NULL ? scaled : bitmap;

	CString ocrText;
	OcrStatus status = RecognizeBitmapText(toOcr, ocrText);

	delete scaled;
	delete bitmap;

	if (status == OCR_STATUS_NO_ENGINE)
	{
		// leave the clip alone, it can be tried again after a restart
		m_engineUnavailable = true;
		Log(_T("ocr engine not available, stopping ocr until restart"));
		return false;
	}

	SaveOcrText(clipId, ocrText);

	Log(StrF(_T("ocr clip %d, text length: %d"), clipId, ocrText.GetLength()));

	return true;
}

void COcrThread::SaveOcrText(int clipId, CString ocrText)
{
	if (ocrText.GetLength() > OCR_MAX_TEXT)
	{
		ocrText = ocrText.Left(OCR_MAX_TEXT);
	}

	try
	{
		CppSQLite3Statement stmt = theApp.m_db.compileStatement(_T("UPDATE Main SET ocrText = ? WHERE lID = ?;"));
		stmt.bind(1, ocrText);
		stmt.bind(2, clipId);
		stmt.execDML();
	}
	CATCH_SQLITE_EXCEPTION
}
