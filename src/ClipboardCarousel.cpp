#include "stdafx.h"
#include "CP_Main.h"
#include "ClipboardCarousel.h"
#include "Options.h"
#include "ProcessPaste.h"
#include "Clip.h"
#include "Misc.h"
#include "ImageHelper.h"
#include <atlimage.h>
#include <shlobj.h>

namespace
{
	const int BATCH_SIZE = 100;
	const size_t CACHE_SIZE = 300;

	bool IsDesktopTarget(HWND window)
	{
		TCHAR className[128] = {};
		GetClassName(window,className,_countof(className));
		CString name(className);
		return window == GetDesktopWindow() || name == _T("Progman") || name == _T("WorkerW");
	}

	bool IsShellWindow(HWND window)
	{
		if(IsDesktopTarget(window)) return true;
		TCHAR className[128] = {};
		GetClassName(window,className,_countof(className));
		CString name(className);
		return name == _T("Shell_TrayWnd") || name == _T("Shell_SecondaryTrayWnd");
	}
}

CClipboardCarousel::CClipboardCarousel()
	: m_owner(NULL), m_notifyWnd(NULL), m_active(false), m_shutdown(false), m_clickPending(false),
	m_target(NULL), m_focus(NULL), m_targetProcess(0), m_clickFocus(NULL), m_clickDeadline(0),
	m_emptyUntil(0), m_previewGeneration(0), m_snapshotOrder(0), m_snapshotId(0), m_current(0),
	m_stop(false), m_requestPending(false)
{
}
CClipboardCarousel::~CClipboardCarousel() { Shutdown(); }
bool CClipboardCarousel::Initialize(CWnd* owner)
{
	m_owner = owner;
	m_notifyWnd = owner->GetSafeHwnd();
	if(!m_window.Create(owner)) return false;
	return m_input.Initialize(m_notifyWnd,m_window.GetSafeHwnd());
}
void CClipboardCarousel::Shutdown()
{
	if(m_shutdown) return;
	m_shutdown = true;
	Cancel(false);
	m_input.Shutdown();
	StopWorker();
	MSG message;
	while(PeekMessage(&message,m_notifyWnd,WM_CLIPBOARD_CAROUSEL_PREVIEW_READY,
		WM_CLIPBOARD_CAROUSEL_PREVIEW_READY,PM_REMOVE))
		HandlePreviewResult((ClipboardCarouselPreviewResult*)message.wParam);
	if(m_window.GetSafeHwnd()) m_window.DestroyWindow();
	m_notifyWnd = NULL;
	m_owner = NULL;
}
void CClipboardCarousel::SuspendInput()
{
	Cancel();
	m_input.Suspend();
	StopWorker();
}
void CClipboardCarousel::ResumeInput()
{
	CString error;
	if(!m_input.Resume(error) && !error.IsEmpty())
		m_owner->MessageBox(error,theApp.m_Language.GetString("ClipboardCarouselTitle","Clipboard Carousel"),MB_OK | MB_ICONWARNING);
	if(!m_input.IsEnabled()) StopWorker();
}
void CClipboardCarousel::HandleCommand(ClipboardCarouselCommand command, UINT_PTR generation)
{
	if(command == ClipboardCarouselCommand::Activate)
	{
		if(!m_active) Start();
		return;
	}
	if(!m_active || generation != m_input.Generation()) return;
	switch(command)
	{
	case ClipboardCarouselCommand::Older: if(!m_clickPending) Move(true); break;
	case ClipboardCarouselCommand::Newer: if(!m_clickPending) Move(false); break;
	case ClipboardCarouselCommand::Confirm: if(!m_clickPending) Confirm(); break;
	case ClipboardCarouselCommand::Cancel: Cancel(); break;
	case ClipboardCarouselCommand::ClickCancel: Cancel(false); break;
	case ClipboardCarouselCommand::ClickPaste:
		if(!CGetSetOptions::GetCarouselPasteAtClick()) Cancel(false);
		else if(!m_clickPending)
		{
			m_clickPending = true;
			m_clickFocus = NULL;
			m_clickDeadline = GetTickCount64() + 1000;
		}
		break;
	default: break;
	}
}
void CClipboardCarousel::Start()
{
	if(m_shutdown || !m_input.IsEnabled() || (theApp.QPasteWnd() && IsWindowVisible(theApp.QPasteWnd()->GetSafeHwnd())))
		return;
	if(!theApp.m_activeWnd.TrackActiveWnd(true)) return;
	m_target = theApp.m_activeWnd.ActiveWnd();
	m_focus = theApp.m_activeWnd.FocusWnd();
	GetWindowThreadProcessId(m_target,&m_targetProcess);
	m_window.RefreshColors();
	m_items.clear();
	m_current = 0;
	if(!LoadBatch(true,true))
	{
		m_window.SetPreview(theApp.m_Language.GetString("ClipboardCarouselEmptyHistory","Clipboard history is empty."));
		m_window.ShowPreview();
		m_emptyUntil = GetTickCount64() + 1000;
		return;
	}
	CString error;
	if(!m_input.BeginSession(error))
	{
		m_owner->MessageBox(error,theApp.m_Language.GetString("ClipboardCarouselTitle","Clipboard Carousel"),MB_OK | MB_ICONWARNING);
		m_items.clear();
		return;
	}
	m_active = true;
	m_emptyUntil = 0;
	RefreshPreview();
}
bool CClipboardCarousel::LoadBatch(bool older, bool initial, bool restart)
{
	try
	{
		if(initial)
		{
			CppSQLite3Query top = theApp.m_db.execQuery(_T("SELECT clipOrder FROM Main INDEXED BY Main_ClipOrder WHERE bIsGroup = 0 ORDER BY clipOrder DESC, lID DESC LIMIT 1"));
			if(top.eof()) return false;
			m_snapshotOrder = top.getFloatField(0);
			m_snapshotId = theApp.m_db.execScalar(_T("SELECT MAX(lID) FROM Main"));
		}
		CString boundary;
		if(restart)
		{
			m_items.clear();
			m_current = 0;
		}
		if(!initial && !restart && !m_items.empty())
		{
			const Item& edge = older ? m_items.back() : m_items.front();
			boundary = StrF(older ? _T("AND clipOrder <= %.17g AND (clipOrder < %.17g OR (clipOrder = %.17g AND lID < %d))") :
				_T("AND clipOrder >= %.17g AND (clipOrder > %.17g OR (clipOrder = %.17g AND lID > %d))"),edge.order,edge.order,edge.order,edge.id);
		}
		CppSQLite3Query query = theApp.m_db.execQueryEx(
			_T("SELECT lID, clipOrder, substr(mText,1,4096) FROM Main INDEXED BY Main_ClipOrder WHERE bIsGroup = 0 AND clipOrder <= %.17g AND lID <= %d %s ORDER BY clipOrder %s, lID %s LIMIT %d"),
			m_snapshotOrder,m_snapshotId,(LPCTSTR)boundary,older ? _T("DESC") : _T("ASC"),older ? _T("DESC") : _T("ASC"),BATCH_SIZE);
		size_t added = 0;
		while(!query.eof())
		{
			Item item = { query.getIntField(0),query.getFloatField(1),query.getStringField(2) };
			if(older) m_items.push_back(item);
			else m_items.push_front(item);
			added++;
			query.nextRow();
		}
		if(restart && added > 0)
			m_current = older ? 0 : m_items.size()-1;
		else if(!older)
			m_current += added;
		while(m_items.size() > CACHE_SIZE)
		{
			if(older) { m_items.pop_front(); m_current--; }
			else m_items.pop_back();
		}
		return added > 0;
	}
	CATCH_SQLITE_EXCEPTION
	return false;
}
bool CClipboardCarousel::ClipExists(int id)
{
	try { return theApp.m_db.execScalar(StrF(_T("SELECT EXISTS(SELECT 1 FROM Main WHERE lID = %d AND bIsGroup = 0)"),id)) != 0; }
	CATCH_SQLITE_EXCEPTION
	return false;
}
void CClipboardCarousel::Move(bool older)
{
	bool wrapped = false;
	if(older)
	{
		if(m_current + 1 >= m_items.size() && !LoadBatch(true))
		{
			if(!LoadBatch(true,false,true)) { Cancel(); return; }
			wrapped = true;
		}
		if(!wrapped) m_current++;
	}
	else
	{
		if(m_current == 0 && !LoadBatch(false))
		{
			if(!LoadBatch(false,false,true)) { Cancel(); return; }
			wrapped = true;
		}
		if(!wrapped) m_current--;
	}
	while(!m_items.empty() && !ClipExists(m_items[m_current].id))
	{
		m_items.erase(m_items.begin() + m_current);
		if(m_items.empty()) { Cancel(); return; }
		if(!older && m_current > 0) m_current--;
		if(m_current >= m_items.size())
		{
			if(!LoadBatch(true)) m_current = m_items.size()-1;
		}
	}
	RefreshPreview();
}
void CClipboardCarousel::RefreshPreview()
{
	if(m_items.empty()) return;
	const Item& item = m_items[m_current];
	CString text = item.description;
	bool image = false, files = false;
	try
	{
		CppSQLite3Query types = theApp.m_db.execQueryEx(_T("SELECT strClipboardFormat FROM Data WHERE lParentID = %d"),item.id);
		while(!types.eof())
		{
			CString type = types.getStringField(0);
			image |= type == GetFormatName(CF_DIB) || type.CompareNoCase(_T("PNG")) == 0;
			files |= type == GetFormatName(CF_HDROP);
			types.nextRow();
		}
		// Only a bounded text prefix is needed for selection and preview.
		CppSQLite3Query plain = theApp.m_db.execQueryEx(
			_T("SELECT strClipboardFormat, substr(ooData,1,8192) FROM Data WHERE lParentID = %d AND strClipboardFormat IN ('CF_UNICODETEXT','CF_TEXT') ORDER BY CASE strClipboardFormat WHEN 'CF_UNICODETEXT' THEN 0 ELSE 1 END LIMIT 1"),item.id);
		if(!plain.eof())
		{
			int length = 0;
			const unsigned char* data = plain.getBlobField(1,length);
			if(data && length > 0)
			{
				if(CString(plain.getStringField(0)) == GetFormatName(CF_UNICODETEXT))
					text = CString((LPCWSTR)data,length / sizeof(WCHAR));
				else
					text = CString(CStringA((LPCSTR)data,length));
				int end = text.Find(_T('\0'));
				if(end >= 0) text = text.Left(end);
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
	if(files)
	{
		image = false;
		HGLOBAL data = CClip::LoadFormat(item.id,CF_HDROP);
		if(data)
		{
			CClipFormat format(CF_HDROP,data);
			text.Empty();
			UINT count = DragQueryFile((HDROP)data,0xFFFFFFFF,NULL,0);
			for(UINT i = 0; i < count && text.GetLength() < 4096; i++)
			{
				CString path;
				UINT length = DragQueryFile((HDROP)data,i,NULL,0);
				DragQueryFile((HDROP)data,i,path.GetBuffer(min(length,4096U)+1),min(length,4096U)+1);
				path.ReleaseBuffer();
				if(i) text += _T("\r\n");
				text += path;
			}
		}
	}
	if(text.IsEmpty()) text = theApp.m_Language.GetString("ClipboardCarouselNoPreview","No preview available.");
	m_previewGeneration++;
	m_window.SetPreview(text,image);
	m_window.ShowPreview();
	if(image)
	{
		StartWorker();
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_request = { item.id,m_previewGeneration,m_window.BackgroundColor() };
			m_requestPending = true;
		}
		m_condition.notify_one();
	}
}
void CClipboardCarousel::Confirm(bool clicked)
{
	if(!m_active || m_items.empty()) return;
	// Desktop activation is useful for browsing, but Enter must not paste files
	// into Explorer's desktop. A destination click establishes an application target.
	if(IsShellWindow(m_target))
	{
		Cancel();
		return;
	}
	int id = m_items[m_current].id;
	if(!ClipExists(id)) { Move(true); return; }
	DWORD process = 0;
	GetWindowThreadProcessId(m_target,&process);
	if(!IsWindow(m_target) || process != m_targetProcess ||
		!theApp.m_activeWnd.RestoreTrackedWnd(m_target,m_focus))
	{
		Cancel(false);
		return;
	}
	CString selected = m_window.SelectedText();
	if(!selected.IsEmpty())
	{
		HGLOBAL data = NewGlobalP((LPVOID)(LPCTSTR)selected,(selected.GetLength()+1)*sizeof(TCHAR));
		HGLOBAL ignore = NewGlobalP("Ignore",sizeof("Ignore"));
		if(!data || !ignore)
		{
			if(data) GlobalFree(data);
			if(ignore) GlobalFree(ignore);
			return;
		}
		EndSession();
		COleDataSource* source = new COleDataSource;
		source->CacheGlobalData(CF_UNICODETEXT,data);
		source->CacheGlobalData(theApp.m_cfIgnoreClipboard,ignore);
		source->SetClipboard();
		if(clicked || CGetSetOptions::m_bSendPasteMessageAfterSelection)
			theApp.m_activeWnd.SendPaste(!clicked);
		else
			theApp.m_activeWnd.ActivateTarget();
		return;
	}
	EndSession();
	CProcessPaste paste;
	paste.GetClipIDs().Add(id);
	paste.m_bSendPaste = clicked || CGetSetOptions::m_bSendPasteMessageAfterSelection;
	paste.m_bActivateTarget = !clicked;
	if(paste.DoPaste()) theApp.OnPasteCompleted();
	else Log(_T("Clipboard Carousel paste failed: ") + paste.m_lastErrorMessage);
}
void CClipboardCarousel::EndSession()
{
	m_active = false;
	m_previewGeneration++;
	m_input.EndSession();
	m_window.HidePreview();
	m_items.clear();
	m_clickPending = false;
	m_emptyUntil = 0;
	std::lock_guard<std::mutex> lock(m_mutex);
	m_requestPending = false;
}
void CClipboardCarousel::Cancel(bool restoreFocus)
{
	bool restore = restoreFocus && m_active && m_window.HasFocus();
	EndSession();
	if(restore && theApp.m_activeWnd.RestoreTrackedWnd(m_target,m_focus))
		theApp.m_activeWnd.ActivateTarget();
}
void CClipboardCarousel::PasteAtClick()
{
	HWND clicked = m_input.ClickedWindow();
	HWND root = GetAncestor(clicked,GA_ROOT);
	if(!IsWindow(clicked) || GetTickCount64() >= m_clickDeadline)
	{
		Cancel(false);
		return;
	}
	bool desktopClick = IsDesktopTarget(root);
	HWND foreground = GetForegroundWindow();
	// The desktop can have different Progman/WorkerW roots across monitors.
	if(foreground != root && !(desktopClick && IsDesktopTarget(foreground))) return;
	POINT point = m_input.ClickedPoint();
	DWORD_PTR hit = 0;
	if(!SendMessageTimeout(root,WM_NCHITTEST,0,MAKELPARAM(point.x,point.y),
		SMTO_ABORTIFHUNG,50,&hit) || hit != HTCLIENT)
	{
		Cancel(false);
		return;
	}
	if(GetAsyncKeyState(VK_LBUTTON) & 0x8000 || GetAsyncKeyState(VK_CONTROL) & 0x8000 ||
		GetAsyncKeyState(VK_SHIFT) & 0x8000 || GetAsyncKeyState(VK_MENU) & 0x8000 ||
		(GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & 0x8000)
		return;
	if(IsShellWindow(root))
	{
		if(desktopClick && IsDesktopTarget(m_target) && m_window.IsImagePreview())
			SaveDesktopImage();
		else
			Cancel(false);
		return;
	}
	if(!theApp.m_activeWnd.TrackActiveWnd(true) || theApp.m_activeWnd.DesktopHasFocus() ||
		theApp.m_activeWnd.ActiveWnd() != root)
	{
		Cancel(false);
		return;
	}
	HWND focus = theApp.m_activeWnd.FocusWnd();
	if(m_clickFocus != focus) { m_clickFocus = focus; return; }
	m_target = root;
	m_focus = focus;
	GetWindowThreadProcessId(root,&m_targetProcess);
	Confirm(true);
}

void CClipboardCarousel::SaveDesktopImage()
{
	m_clickPending = false;
	if(m_items.empty() || !ClipExists(m_items[m_current].id))
	{
		Cancel(false);
		return;
	}
	bool saved = false;
	CString savedPath;
	do
	{
		PWSTR desktop = NULL;
		HRESULT result = SHGetKnownFolderPath(FOLDERID_Desktop,KF_FLAG_DEFAULT,NULL,&desktop);
		CString folder = desktop ? desktop : _T("");
		CoTaskMemFree(desktop);
		if(FAILED(result) || folder.IsEmpty()) break;

		// Decode the original saved format, never the downscaled preview bitmap.
		CClipFormat format(theApp.m_PNG_Format);
		if(!theApp.GetClipData(m_items[m_current].id,format))
		{
			format.Free(); format.Clear();
			format.m_cfType = CF_DIB;
			theApp.GetClipData(m_items[m_current].id,format);
		}
		if(!format.m_hgData) break;
		std::shared_ptr<CImage> image = format.m_cfType == theApp.m_PNG_Format ?
			PNGImageHelper::CImageFromHGLOBAL(format.m_hgData) :
			DIBImageHelper::CImageFromHGLOBAL(format.m_hgData);
		if(!image || image->IsNull()) break;
		CComPtr<IStream> encoded;
		if(FAILED(CreateStreamOnHGlobal(NULL,TRUE,&encoded)) ||
			FAILED(image->Save(encoded,Gdiplus::ImageFormatPNG))) break;
		STATSTG info = {};
		HGLOBAL buffer = NULL;
		if(FAILED(encoded->Stat(&info,STATFLAG_NONAME)) || info.cbSize.HighPart != 0 ||
			info.cbSize.LowPart == 0 || FAILED(GetHGlobalFromStream(encoded,&buffer))) break;
		BYTE* bytes = (BYTE*)GlobalLock(buffer);
		if(!bytes) break;

		SYSTEMTIME time = {};
		GetLocalTime(&time);
		CString stem = StrF(_T("Ditto image %04d-%02d-%02d %02d-%02d-%02d"),
			time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond);
		for(int suffix = 0; suffix < 1000; suffix++)
		{
			CString name = stem + (suffix ? StrF(_T(" (%d)"),suffix+1) : CString()) + _T(".png");
			savedPath = folder + _T("\\") + name;
			HANDLE handle = CreateFile(savedPath,GENERIC_WRITE | DELETE,0,NULL,
				CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
			if(handle == INVALID_HANDLE_VALUE)
			{
				DWORD error = GetLastError();
				if(error == ERROR_FILE_EXISTS || error == ERROR_ALREADY_EXISTS) continue;
				break;
			}
			CHandle file(handle);
			DWORD offset = 0;
			bool written = true;
			while(offset < info.cbSize.LowPart)
			{
				DWORD count = min(info.cbSize.LowPart-offset,1024UL*1024);
				DWORD actual = 0;
				if(!WriteFile(file,bytes+offset,count,&actual,NULL) || actual == 0)
				{
					written = false;
					break;
				}
				offset += actual;
			}
			saved = written && FlushFileBuffers(file) != FALSE;
			if(!saved)
			{
				// Remove only the failed new file, through its exclusive handle.
				FILE_DISPOSITION_INFO discard = { TRUE };
				SetFileInformationByHandle(file,FileDispositionInfo,&discard,sizeof(discard));
			}
			break;
		}
		GlobalUnlock(buffer);
	} while(false);
	// Export does not load the clipboard or mark/reorder the original history clip.
	EndSession();
	if(saved)
		SHChangeNotify(SHCNE_CREATE,SHCNF_PATH,(LPCTSTR)savedPath,NULL);
	else
		m_owner->MessageBox(theApp.m_Language.GetString("ClipboardCarouselDesktopSaveFailed",
			"Unable to save the image to the Desktop. Check that the image is valid and the Desktop folder is writable."),
			theApp.m_Language.GetString("ClipboardCarouselTitle","Clipboard Carousel"),MB_OK | MB_ICONWARNING);
}
void CClipboardCarousel::Tick()
{
	m_input.Tick();
	if(m_emptyUntil && GetTickCount64() >= m_emptyUntil)
	{
		m_emptyUntil = 0;
		m_window.HidePreview();
	}
	if(m_active && m_clickPending) PasteAtClick();
}
void CClipboardCarousel::BeforeDatabaseClose() { Cancel(); StopWorker(); }
void CClipboardCarousel::StartWorker()
{
	if(!m_worker.joinable())
	{
		m_stop = false;
		m_worker = std::thread(&CClipboardCarousel::PreviewWorker,this);
	}
}
void CClipboardCarousel::StopWorker()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_stop = true;
		m_requestPending = false;
	}
	m_condition.notify_one();
	if(m_worker.joinable()) m_worker.join();
}
void CClipboardCarousel::PreviewWorker()
{
	while(true)
	{
		PreviewRequest request;
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_condition.wait(lock,[&]() { return m_stop || m_requestPending; });
			if(m_stop) return;
			request = m_request;
			m_requestPending = false;
		}
		CClipFormat format(CF_DIB);
		if(!theApp.GetClipData(request.id,format))
		{
			format.Free(); format.Clear();
			format.m_cfType = theApp.m_PNG_Format;
			theApp.GetClipData(request.id,format);
		}
		HBITMAP image = NULL;
		if(format.m_hgData)
		{
			Gdiplus::Bitmap* original = format.CreateGdiplusBitmap();
			if(original && original->GetWidth() && original->GetHeight())
			{
				double scale = min(1.0,min(720.0/original->GetWidth(),420.0/original->GetHeight()));
				INT width = max(1,(INT)(original->GetWidth()*scale));
				INT height = max(1,(INT)(original->GetHeight()*scale));
				Gdiplus::Bitmap thumbnail(width,height,PixelFormat32bppPARGB);
				{
					Gdiplus::Graphics graphics(&thumbnail);
					graphics.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
					graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
					graphics.DrawImage(original,0,0,width,height);
				}
				thumbnail.GetHBITMAP(Gdiplus::Color(255,GetRValue(request.background),
					GetGValue(request.background),GetBValue(request.background)),&image);
			}
			delete original;
		}
		ClipboardCarouselPreviewResult* result = new ClipboardCarouselPreviewResult;
		result->generation = request.generation;
		result->clipId = request.id;
		result->image = image;
		if(!PostMessage(m_notifyWnd,WM_CLIPBOARD_CAROUSEL_PREVIEW_READY,(WPARAM)result,0))
		{
			if(image) DeleteObject(image);
			delete result;
		}
	}
}
void CClipboardCarousel::HandlePreviewResult(ClipboardCarouselPreviewResult* result)
{
	if(!result) return;
	if(m_active && !m_items.empty() && result->generation == m_previewGeneration && result->clipId == m_items[m_current].id)
	{
		m_window.SetImage(result->image);
		result->image = NULL;
	}
	if(result->image) DeleteObject(result->image);
	delete result;
}
