#pragma once

#include "ClipboardCarouselInput.h"
#include "ClipboardCarouselWnd.h"
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

struct ClipboardCarouselPreviewResult
{
	ULONGLONG generation = 0;
	int clipId = -1;
	HBITMAP image = NULL;
};

class CClipboardCarousel
{
public:
	CClipboardCarousel();
	~CClipboardCarousel();
	bool Initialize(CWnd* owner);
	void Shutdown();
	void SuspendInput();
	void ResumeInput();
	bool HandleHotKey(WPARAM id) { return m_input.HandleHotKey(id); }
	void HandleCommand(ClipboardCarouselCommand command, UINT_PTR generation);
	void HandlePreviewResult(ClipboardCarouselPreviewResult* result);
	void Tick();
	void Cancel(bool restoreFocus = true);
	void BeforeDatabaseClose();
	void RefreshColors() { m_window.RefreshColors(); }
	bool IsActive() const { return m_active; }
	bool IsEnabled() const { return m_input.IsEnabled(); }

private:
	struct Item { int id; double order; CString description; };
	struct PreviewRequest { int id = -1; ULONGLONG generation = 0; COLORREF background = RGB(32,32,32); };
	void Start();
	bool LoadBatch(bool older, bool initial = false, bool restart = false);
	void Move(bool older);
	bool ClipExists(int id);
	void RefreshPreview();
	void Confirm(bool clicked = false);
	void EndSession();
	void PasteAtClick();
	void SaveDesktopImage();
	void StartWorker();
	void StopWorker();
	void PreviewWorker();

	CWnd* m_owner;
	HWND m_notifyWnd;
	CClipboardCarouselInput m_input;
	CClipboardCarouselWnd m_window;
	bool m_active;
	bool m_shutdown;
	bool m_clickPending;
	HWND m_target;
	HWND m_focus;
	DWORD m_targetProcess;
	HWND m_clickFocus;
	ULONGLONG m_clickDeadline;
	ULONGLONG m_emptyUntil;
	ULONGLONG m_previewGeneration;
	double m_snapshotOrder;
	int m_snapshotId;
	std::deque<Item> m_items;
	size_t m_current;
	std::thread m_worker;
	std::mutex m_mutex;
	std::condition_variable m_condition;
	bool m_stop;
	bool m_requestPending;
	PreviewRequest m_request;
};
