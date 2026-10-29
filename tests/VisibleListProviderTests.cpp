#include <windows.h>
#include <commctrl.h>
#include <UIAutomation.h>
#include <cassert>
#include <cstdio>
#include <crtdbg.h>
#include <atomic>
#include <thread>
#include <chrono>
#include "../src/VisibleListProvider.h"

static int invoked = 0;
static int clipIdOffset = 0;
static bool reorderOnAction = false;
static LRESULT CALLBACK ListSubclass(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR, DWORD_PTR)
{
    if (message >= WM_VISIBLE_LIST_UIA_ACTION && message <= WM_VISIBLE_LIST_UIA_ACTION + static_cast<UINT>(VisibleListAction::Invoke))
    {
        if (reorderOnAction) { ++clipIdOffset; reorderOnAction = false; }
        return HandleVisibleListAction(hwnd, static_cast<VisibleListAction>(message - WM_VISIBLE_LIST_UIA_ACTION), static_cast<int>(wparam), static_cast<LONG>(lparam));
    }
    if (message == WM_GETOBJECT && static_cast<LONG>(lparam) == UiaRootObjectId)
    {
        auto provider = new VisibleListProvider(hwnd);
        auto result = UiaReturnRawElementProvider(hwnd, wparam, lparam, static_cast<IRawElementProviderSimple *>(provider));
        provider->Release();
        return result;
    }
    return DefSubclassProc(hwnd, message, wparam, lparam);
}
static LRESULT CALLBACK ParentProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_NOTIFY)
    {
        auto header = reinterpret_cast<NMHDR *>(lparam);
        if (header->code == LVN_GETDISPINFOW)
        {
            auto info = reinterpret_cast<NMLVDISPINFOW *>(lparam);
            if (info->item.mask & LVIF_TEXT)
                _snwprintf_s(info->item.pszText, info->item.cchTextMax, _TRUNCATE, L"clip %d", info->item.iItem);
            if (info->item.mask & LVIF_PARAM) info->item.lParam = 1000 + info->item.iItem + clipIdOffset;
            return 0;
        }
        if (header->code == NM_DBLCLK) { ++invoked; return 0; }
    }
    return DefWindowProc(hwnd, message, wparam, lparam);
}

int main()
{
    _set_error_mode(_OUT_TO_STDERR);
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    INITCOMMONCONTROLSEX cc{sizeof(cc), ICC_LISTVIEW_CLASSES};
    assert(InitCommonControlsEx(&cc));
    WNDCLASSW wc{}; wc.lpfnWndProc = ParentProc; wc.hInstance = GetModuleHandle(nullptr); wc.lpszClassName = L"DittoUiaFixture";
    assert(RegisterClassW(&wc));
    HWND parent = CreateWindowW(wc.lpszClassName, L"", WS_OVERLAPPED, 0, 0, 400, 400, nullptr, nullptr, wc.hInstance, nullptr);
    assert(parent);
    HWND list = CreateWindowW(WC_LISTVIEWW, L"", WS_CHILD | LVS_REPORT | LVS_OWNERDATA, 0, 0, 300, 200, parent, nullptr, wc.hInstance, nullptr);
    assert(list);
    LVCOLUMNW column{}; column.mask = LVCF_WIDTH; column.cx = 200;
    assert(ListView_InsertColumn(list, 0, &column) == 0);
    assert(ListView_SetItemCountEx(list, 216025, 0));
    assert(SetWindowSubclass(list, ListSubclass, 1, 0));
    auto root = new VisibleListProvider(list);
    IRawElementProviderFragment *outside = nullptr;
    assert(SUCCEEDED(root->ElementProviderFromPoint(-10000, -10000, &outside)) && !outside);
    IRawElementProviderFragment *item = nullptr;
    assert(SUCCEEDED(root->Navigate(NavigateDirection_FirstChild, &item)) && item);
    int visited = 0;
    IRawElementProviderFragment *firstItem = item;
    firstItem->AddRef();
    while (item)
    {
        ++visited;
        IRawElementProviderSimple *simple = nullptr;
        assert(SUCCEEDED(item->QueryInterface(IID_PPV_ARGS(&simple))));
        VARIANT value; VariantInit(&value);
        assert(SUCCEEDED(simple->GetPropertyValue(UIA_NamePropertyId, &value)) && value.vt == VT_BSTR && SysStringLen(value.bstrVal));
        VariantClear(&value); simple->Release();
        IRawElementProviderFragment *next = nullptr;
        assert(SUCCEEDED(item->Navigate(NavigateDirection_NextSibling, &next)));
        item->Release(); item = next;
        assert(visited < 100);
    }
    assert(visited > 0 && visited < 100);
    assert(ListView_GetItemCount(list) == 216025);
    assert(SUCCEEDED(root->SetScrollPercent(UIA_ScrollPatternNoScroll, 50)));
    assert(ListView_GetTopIndex(list) > 1000);
    VARIANT stale; VariantInit(&stale);
    IRawElementProviderSimple *old = nullptr;
    assert(SUCCEEDED(firstItem->QueryInterface(IID_PPV_ARGS(&old))));
    assert(old->GetPropertyValue(UIA_NamePropertyId, &stale) == UIA_E_ELEMENTNOTAVAILABLE);
    IVirtualizedItemProvider *virtualItem = nullptr;
    assert(SUCCEEDED(firstItem->QueryInterface(IID_PPV_ARGS(&virtualItem))));
    assert(SUCCEEDED(virtualItem->Realize()));
    assert(ListView_GetTopIndex(list) < 10);
    assert(SUCCEEDED(old->GetPropertyValue(UIA_NamePropertyId, &stale)) && stale.vt == VT_BSTR);
    VariantClear(&stale); virtualItem->Release();
    old->Release(); firstItem->Release();
    assert(SUCCEEDED(root->SetScrollPercent(UIA_ScrollPatternNoScroll, 50)));
    IRawElementProviderFragment *middle = nullptr;
    assert(SUCCEEDED(root->Navigate(NavigateDirection_FirstChild, &middle)) && middle);
    ISelectionItemProvider *selection = nullptr;
    assert(SUCCEEDED(middle->QueryInterface(IID_PPV_ARGS(&selection))));
    assert(SUCCEEDED(middle->SetFocus()));
    assert(ListView_GetNextItem(list, -1, LVNI_FOCUSED) == ListView_GetTopIndex(list));
    assert(SUCCEEDED(selection->Select()));
    BOOL selected = FALSE; assert(SUCCEEDED(selection->get_IsSelected(&selected)) && selected);
    IRawElementProviderFragment *nextSelection = nullptr;
    assert(SUCCEEDED(middle->Navigate(NavigateDirection_NextSibling, &nextSelection)) && nextSelection);
    ISelectionItemProvider *extra = nullptr;
    assert(SUCCEEDED(nextSelection->QueryInterface(IID_PPV_ARGS(&extra))));
    assert(SUCCEEDED(extra->AddToSelection()));
    SAFEARRAY *selectedItems = nullptr;
    assert(SUCCEEDED(root->GetSelection(&selectedItems)) && selectedItems);
    LONG lower = 0, upper = 0;
    assert(SUCCEEDED(SafeArrayGetLBound(selectedItems, 1, &lower)));
    assert(SUCCEEDED(SafeArrayGetUBound(selectedItems, 1, &upper)) && upper - lower + 1 == 2);
    SafeArrayDestroy(selectedItems);
    extra->Release(); nextSelection->Release();
    IInvokeProvider *invoke = nullptr;
    assert(SUCCEEDED(middle->QueryInterface(IID_PPV_ARGS(&invoke))));
    reorderOnAction = true;
    assert(invoke->Invoke() == UIA_E_ELEMENTNOTAVAILABLE && invoked == 0);
    --clipIdOffset;
    assert(SUCCEEDED(invoke->Invoke()) && invoked == 1);
    invoke->Release(); selection->Release(); middle->Release(); root->Release();

    std::atomic<int> status{0};
    std::thread client([&] {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        IUIAutomation *uia = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&uia));
        IUIAutomationElement *element = nullptr;
        if (SUCCEEDED(hr)) hr = uia->ElementFromHandle(list, &element);
        IUIAutomationCondition *condition = nullptr;
        if (SUCCEEDED(hr)) hr = uia->CreateTrueCondition(&condition);
        IUIAutomationElementArray *children = nullptr;
        if (SUCCEEDED(hr)) hr = element->FindAll(TreeScope_Children, condition, &children);
        int length = 0;
        if (SUCCEEDED(hr)) hr = children->get_Length(&length);
        IUIAutomationElement *firstVisible = nullptr;
        if (SUCCEEDED(hr) && length > 0 && length < 100) hr = children->GetElement(0, &firstVisible);
        else if (SUCCEEDED(hr)) hr = E_FAIL;
        BSTR itemName = nullptr;
        if (SUCCEEDED(hr)) hr = firstVisible->get_CurrentName(&itemName);
        if (SUCCEEDED(hr) && (!itemName || !SysStringLen(itemName))) hr = E_FAIL;
        if (itemName) SysFreeString(itemName);
        IUnknown *selectionUnknown = nullptr;
        if (SUCCEEDED(hr)) hr = firstVisible->GetCurrentPattern(UIA_SelectionItemPatternId, &selectionUnknown);
        IUIAutomationSelectionItemPattern *selectionPattern = nullptr;
        if (SUCCEEDED(hr) && selectionUnknown) hr = selectionUnknown->QueryInterface(IID_PPV_ARGS(&selectionPattern));
        else if (SUCCEEDED(hr)) hr = E_FAIL;
        if (SUCCEEDED(hr)) hr = selectionPattern->Select();
        IUnknown *invokeUnknown = nullptr;
        if (SUCCEEDED(hr)) hr = firstVisible->GetCurrentPattern(UIA_InvokePatternId, &invokeUnknown);
        IUIAutomationInvokePattern *invokePattern = nullptr;
        if (SUCCEEDED(hr) && invokeUnknown) hr = invokeUnknown->QueryInterface(IID_PPV_ARGS(&invokePattern));
        else if (SUCCEEDED(hr)) hr = E_FAIL;
        if (SUCCEEDED(hr)) hr = invokePattern->Invoke();
        if (invokePattern) invokePattern->Release();
        if (invokeUnknown) invokeUnknown->Release();
        if (selectionPattern) selectionPattern->Release();
        if (selectionUnknown) selectionUnknown->Release();
        if (firstVisible) firstVisible->Release();
        if (children) children->Release();
        if (condition) condition->Release();
        if (element) element->Release();
        if (uia) uia->Release();
        CoUninitialize();
        status = SUCCEEDED(hr) && length > 0 && length < 100 ? length : -1;
    });
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
    while (!status && std::chrono::steady_clock::now() < deadline)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
        MsgWaitForMultipleObjectsEx(0, nullptr, 20, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
    }
    if (!status) { std::fprintf(stderr, "FAIL: UI Automation timed out\n"); ExitProcess(3); }
    client.join();
    if (status < 0) { std::fprintf(stderr, "FAIL: UI Automation child query\n"); return 2; }
    assert(invoked == 2);
    std::printf("PASS: UI Automation returned %d viewport children\n", status.load());
    DestroyWindow(parent); CoUninitialize();
    std::printf("PASS: bounded visible children, full count, scroll, stale item, selection, invoke\n");
}
