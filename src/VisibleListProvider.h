#pragma once

#include <UIAutomation.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

constexpr UINT WM_VISIBLE_LIST_UIA_ACTION = WM_APP + 4100;
enum class VisibleListAction { Focus, Select, Add, Remove, Invoke };

inline LONG VisibleListClipId(HWND hwnd, int row)
{
    NMLVDISPINFOW info{};
    info.hdr.hwndFrom = hwnd; info.hdr.idFrom = GetDlgCtrlID(hwnd); info.hdr.code = LVN_GETDISPINFOW;
    info.item.iItem = row; info.item.mask = LVIF_PARAM; info.item.lParam = -1;
    ::SendMessageW(::GetParent(hwnd), WM_NOTIFY, info.hdr.idFrom, reinterpret_cast<LPARAM>(&info));
    return static_cast<LONG>(info.item.lParam);
}

// Executed on the list's UI thread so a reorder cannot occur between the
// identity check and an accessibility action.
inline LRESULT HandleVisibleListAction(HWND hwnd, VisibleListAction action, int row, LONG clipId)
{
    if (row >= 0 && (row >= static_cast<int>(::SendMessage(hwnd, LVM_GETITEMCOUNT, 0, 0)) ||
        clipId <= 0 || VisibleListClipId(hwnd, row) != clipId)) return UIA_E_ELEMENTNOTAVAILABLE;
    if (action == VisibleListAction::Focus)
    {
        if (row >= 0)
        {
            LVITEMW item{}; item.stateMask = LVIS_FOCUSED;
            ::SendMessage(hwnd, LVM_SETITEMSTATE, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(&item));
            item.state = LVIS_FOCUSED;
            ::SendMessage(hwnd, LVM_SETITEMSTATE, row, reinterpret_cast<LPARAM>(&item));
            ::SendMessage(hwnd, LVM_ENSUREVISIBLE, row, FALSE);
        }
        ::SetFocus(hwnd);
        return S_OK;
    }
    if (row < 0) return E_INVALIDARG;
    if (action == VisibleListAction::Add && (GetWindowLongPtr(hwnd, GWL_STYLE) & LVS_SINGLESEL) &&
        ::SendMessage(hwnd, LVM_GETSELECTEDCOUNT, 0, 0) &&
        !(::SendMessage(hwnd, LVM_GETITEMSTATE, row, LVIS_SELECTED) & LVIS_SELECTED)) return UIA_E_INVALIDOPERATION;
    LVITEMW item{};
    if (action == VisibleListAction::Select || action == VisibleListAction::Invoke)
    {
        item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        ::SendMessage(hwnd, LVM_SETITEMSTATE, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(&item));
        item.state = LVIS_SELECTED | LVIS_FOCUSED;
    }
    else if (action == VisibleListAction::Add) { item.stateMask = LVIS_SELECTED; item.state = LVIS_SELECTED; }
    else if (action == VisibleListAction::Remove) { item.stateMask = LVIS_SELECTED; }
    else return E_INVALIDARG;
    ::SendMessage(hwnd, LVM_SETITEMSTATE, row, reinterpret_cast<LPARAM>(&item));
    if (action == VisibleListAction::Invoke)
    {
        NMITEMACTIVATE event{}; event.hdr.hwndFrom = hwnd; event.hdr.idFrom = GetDlgCtrlID(hwnd); event.hdr.code = NM_DBLCLK; event.iItem = row;
        ::SendMessage(::GetParent(hwnd), WM_NOTIFY, event.hdr.idFrom, reinterpret_cast<LPARAM>(&event));
    }
    return S_OK;
}

// The system list-view proxy enumerates the entire owner-data list when UIA
// clients subscribe to focus events. Expose only the viewport in the UIA tree;
// scrolling and the list's normal keyboard/mouse navigation remain available.
class VisibleListProvider final : public IRawElementProviderSimple,
                                  public IRawElementProviderFragment,
                                  public IRawElementProviderFragmentRoot,
                                  public ISelectionProvider,
                                  public ISelectionItemProvider,
                                  public IInvokeProvider,
                                  public IScrollProvider,
                                  public IVirtualizedItemProvider
{
    LONG references = 1;
    HWND hwnd;
    int row;
    VisibleListProvider *root;
    LONG clipId = -1;

    bool valid() const { return ::IsWindow(hwnd) != FALSE; }
    int count() const { return static_cast<int>(::SendMessage(hwnd, LVM_GETITEMCOUNT, 0, 0)); }
    int first() const { return static_cast<int>(::SendMessage(hwnd, LVM_GETTOPINDEX, 0, 0)); }
    int page() const { return (std::max)(1, static_cast<int>(::SendMessage(hwnd, LVM_GETCOUNTPERPAGE, 0, 0)) + 1); }
    int last() const { return (std::min)(count() - 1, first() + page() - 1); }
    bool visible() const { return row >= first() && row <= last(); }
    LONG currentClipId() const { return VisibleListClipId(hwnd, row); }
    // A saved automation element must not invoke a different clip after the
    // sorted list changes. Unloaded rows become available after realization.
    bool stable()
    {
        if (!valid() || row < 0 || row >= count() || !visible()) return false;
        LONG current = currentClipId();
        if (current <= 0) return false;
        InterlockedCompareExchange(&clipId, current, -1);
        return InterlockedCompareExchange(&clipId, 0, 0) == current;
    }
    VisibleListProvider *child(int index) { return new VisibleListProvider(hwnd, index, this); }
    HRESULT perform(VisibleListAction action)
    {
        if (!valid() || (row >= 0 && !stable())) return UIA_E_ELEMENTNOTAVAILABLE;
        return static_cast<HRESULT>(::SendMessage(hwnd, WM_VISIBLE_LIST_UIA_ACTION + static_cast<UINT>(action),
            static_cast<WPARAM>(row), static_cast<LPARAM>(clipId)));
    }
public:
    explicit VisibleListProvider(HWND window, int index = -1, VisibleListProvider *parent = nullptr)
        : hwnd(window), row(index), root(parent)
    {
        if (root)
        {
            root->AddRef();
            if (valid() && visible()) { LONG current = currentClipId(); if (current > 0) clipId = current; }
        }
    }
    ~VisibleListProvider() { if (root) root->Release(); }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **out) override
    {
        if (!out) return E_POINTER;
        *out = nullptr;
        if (iid == __uuidof(IUnknown) || iid == __uuidof(IRawElementProviderSimple))
            *out = static_cast<IRawElementProviderSimple *>(this);
        else if (iid == __uuidof(IRawElementProviderFragment))
            *out = static_cast<IRawElementProviderFragment *>(this);
        else if (row < 0 && iid == __uuidof(IRawElementProviderFragmentRoot))
            *out = static_cast<IRawElementProviderFragmentRoot *>(this);
        else if (row < 0 && iid == __uuidof(IScrollProvider))
            *out = static_cast<IScrollProvider *>(this);
        else if (row < 0 && iid == __uuidof(ISelectionProvider))
            *out = static_cast<ISelectionProvider *>(this);
        else if (row >= 0 && iid == __uuidof(ISelectionItemProvider))
            *out = static_cast<ISelectionItemProvider *>(this);
        else if (row >= 0 && iid == __uuidof(IInvokeProvider))
            *out = static_cast<IInvokeProvider *>(this);
        else if (row >= 0 && iid == __uuidof(IVirtualizedItemProvider))
            *out = static_cast<IVirtualizedItemProvider *>(this);
        if (!*out) return E_NOINTERFACE;
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&references); }
    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG remaining = InterlockedDecrement(&references);
        if (!remaining) delete this;
        return remaining;
    }
    HRESULT STDMETHODCALLTYPE get_ProviderOptions(ProviderOptions *out) override
    {
        if (!out) return E_POINTER;
        *out = ProviderOptions_ServerSideProvider;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetPatternProvider(PATTERNID pattern, IUnknown **out) override
    {
        if (!out) return E_POINTER;
        *out = nullptr;
        if (row < 0 && pattern == UIA_ScrollPatternId) *out = static_cast<IScrollProvider *>(this);
        else if (row < 0 && pattern == UIA_SelectionPatternId) *out = static_cast<ISelectionProvider *>(this);
        else if (row >= 0 && pattern == UIA_SelectionItemPatternId) *out = static_cast<ISelectionItemProvider *>(this);
        else if (row >= 0 && pattern == UIA_InvokePatternId) *out = static_cast<IInvokeProvider *>(this);
        else if (row >= 0 && pattern == UIA_VirtualizedItemPatternId) *out = static_cast<IVirtualizedItemProvider *>(this);
        if (*out) AddRef();
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetPropertyValue(PROPERTYID property, VARIANT *out) override
    {
        if (!out) return E_POINTER;
        VariantInit(out);
        if (!valid() || (row >= 0 && row >= count())) return UIA_E_ELEMENTNOTAVAILABLE;
        if (property == UIA_IsOffscreenPropertyId)
        {
            out->vt = VT_BOOL; out->boolVal = row >= 0 && !visible() ? VARIANT_TRUE : VARIANT_FALSE;
            return S_OK;
        }
        if (row >= 0 && !stable()) return UIA_E_ELEMENTNOTAVAILABLE;
        if (property == UIA_ControlTypePropertyId) { out->vt = VT_I4; out->lVal = row < 0 ? UIA_ListControlTypeId : UIA_ListItemControlTypeId; }
        else if (property == UIA_IsControlElementPropertyId || property == UIA_IsContentElementPropertyId || property == UIA_IsEnabledPropertyId || property == UIA_IsKeyboardFocusablePropertyId)
        { out->vt = VT_BOOL; out->boolVal = VARIANT_TRUE; }
        else if (property == UIA_HasKeyboardFocusPropertyId)
        {
            out->vt = VT_BOOL;
            GUITHREADINFO info{}; info.cbSize = sizeof(info);
            bool focused = ::GetGUIThreadInfo(::GetWindowThreadProcessId(hwnd, nullptr), &info) && info.hwndFocus == hwnd &&
                (row < 0 || (::SendMessage(hwnd, LVM_GETITEMSTATE, row, LVIS_FOCUSED) & LVIS_FOCUSED));
            out->boolVal = focused ? VARIANT_TRUE : VARIANT_FALSE;
        }
        else if (property == UIA_NamePropertyId)
        {
            out->vt = VT_BSTR;
            if (row < 0) out->bstrVal = SysAllocString(L"Clipboard history");
            else
            {
                wchar_t text[2048]{};
                LVITEMW item{}; item.iSubItem = 0; item.pszText = text; item.cchTextMax = _countof(text);
                ::SendMessageW(hwnd, LVM_GETITEMTEXTW, row, reinterpret_cast<LPARAM>(&item));
                out->bstrVal = SysAllocString(text);
            }
            if (!out->bstrVal) return E_OUTOFMEMORY;
        }
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_HostRawElementProvider(IRawElementProviderSimple **out) override
    {
        if (!out) return E_POINTER;
        *out = nullptr;
        return row < 0 ? UiaHostProviderFromHwnd(hwnd, out) : S_OK;
    }
    HRESULT STDMETHODCALLTYPE Navigate(NavigateDirection direction, IRawElementProviderFragment **out) override
    {
        if (!out) return E_POINTER;
        *out = nullptr;
        if (!valid()) return UIA_E_ELEMENTNOTAVAILABLE;
        if (row < 0 && count() > 0 && (direction == NavigateDirection_FirstChild || direction == NavigateDirection_LastChild))
            *out = static_cast<IRawElementProviderFragment *>(child(direction == NavigateDirection_FirstChild ? first() : last()));
        else if (row >= 0 && direction == NavigateDirection_Parent)
        { root->AddRef(); *out = static_cast<IRawElementProviderFragment *>(root); }
        else if (row >= 0 && visible() && direction == NavigateDirection_NextSibling && row < last())
            *out = static_cast<IRawElementProviderFragment *>(root->child(row + 1));
        else if (row >= 0 && visible() && direction == NavigateDirection_PreviousSibling && row > first())
            *out = static_cast<IRawElementProviderFragment *>(root->child(row - 1));
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetRuntimeId(SAFEARRAY **out) override
    {
        if (!out) return E_POINTER;
        *out = nullptr;
        if (row < 0) return S_OK;
        if (!stable()) return UIA_E_ELEMENTNOTAVAILABLE;
        SAFEARRAY *array = SafeArrayCreateVector(VT_I4, 0, 3);
        if (!array) return E_OUTOFMEMORY;
        LONG values[] = { UiaAppendRuntimeId, static_cast<LONG>(reinterpret_cast<ULONG_PTR>(hwnd)), clipId };
        for (LONG i = 0; i < 3; ++i) SafeArrayPutElement(array, &i, &values[i]);
        *out = array;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_BoundingRectangle(UiaRect *out) override
    {
        if (!out) return E_POINTER;
        *out = {};
        if (!valid() || (row >= 0 && !stable())) return UIA_E_ELEMENTNOTAVAILABLE;
        RECT rect{};
        if (row < 0) ::GetWindowRect(hwnd, &rect);
        else
        {
            rect.left = LVIR_BOUNDS;
            if (!::SendMessage(hwnd, LVM_GETITEMRECT, row, reinterpret_cast<LPARAM>(&rect))) return UIA_E_ELEMENTNOTAVAILABLE;
            POINT upper{rect.left, rect.top}, lower{rect.right, rect.bottom};
            ::ClientToScreen(hwnd, &upper); ::ClientToScreen(hwnd, &lower);
            rect = {upper.x, upper.y, lower.x, lower.y};
        }
        out->left = rect.left; out->top = rect.top; out->width = rect.right - rect.left; out->height = rect.bottom - rect.top;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetEmbeddedFragmentRoots(SAFEARRAY **out) override { if (!out) return E_POINTER; *out = nullptr; return S_OK; }
    HRESULT STDMETHODCALLTYPE SetFocus() override { return perform(VisibleListAction::Focus); }
    HRESULT STDMETHODCALLTYPE get_FragmentRoot(IRawElementProviderFragmentRoot **out) override
    {
        if (!out) return E_POINTER;
        auto provider = root ? root : this;
        provider->AddRef(); *out = static_cast<IRawElementProviderFragmentRoot *>(provider);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE ElementProviderFromPoint(double x, double y, IRawElementProviderFragment **out) override
    {
        if (!out) return E_POINTER;
        *out = nullptr;
        if (!valid()) return UIA_E_ELEMENTNOTAVAILABLE;
        LVHITTESTINFO hit{}; hit.pt = {static_cast<LONG>(x), static_cast<LONG>(y)};
        ::ScreenToClient(hwnd, &hit.pt);
        RECT client{};
        ::GetClientRect(hwnd, &client);
        if (!::PtInRect(&client, hit.pt)) return S_OK;
        int index = static_cast<int>(::SendMessage(hwnd, LVM_HITTEST, 0, reinterpret_cast<LPARAM>(&hit)));
        if (index >= first() && index <= last()) *out = static_cast<IRawElementProviderFragment *>(child(index));
        else { AddRef(); *out = static_cast<IRawElementProviderFragment *>(this); }
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetFocus(IRawElementProviderFragment **out) override
    {
        if (!out) return E_POINTER;
        *out = nullptr;
        if (!valid()) return UIA_E_ELEMENTNOTAVAILABLE;
        int index = static_cast<int>(::SendMessage(hwnd, LVM_GETNEXTITEM, static_cast<WPARAM>(-1), LVNI_FOCUSED));
        if (index >= first() && index <= last()) *out = static_cast<IRawElementProviderFragment *>(child(index));
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetSelection(SAFEARRAY **out) override
    {
        if (!out) return E_POINTER;
        *out = nullptr;
        if (!valid()) return UIA_E_ELEMENTNOTAVAILABLE;
        std::vector<int> selected;
        int index = -1;
        while ((index = static_cast<int>(::SendMessage(hwnd, LVM_GETNEXTITEM, static_cast<WPARAM>(index), LVNI_SELECTED))) >= 0)
            selected.push_back(index);
        SAFEARRAY *array = SafeArrayCreateVector(VT_UNKNOWN, 0, static_cast<ULONG>(selected.size()));
        if (!array) return E_OUTOFMEMORY;
        for (LONG i = 0; i < static_cast<LONG>(selected.size()); ++i)
        {
            auto item = child(selected[i]);
            HRESULT hr = SafeArrayPutElement(array, &i, static_cast<IRawElementProviderSimple *>(item));
            item->Release();
            if (FAILED(hr)) { SafeArrayDestroy(array); return hr; }
        }
        *out = array;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_CanSelectMultiple(BOOL *out) override { if (!out) return E_POINTER; *out = !(GetWindowLongPtr(hwnd, GWL_STYLE) & LVS_SINGLESEL); return S_OK; }
    HRESULT STDMETHODCALLTYPE get_IsSelectionRequired(BOOL *out) override { if (!out) return E_POINTER; *out = FALSE; return S_OK; }
    HRESULT STDMETHODCALLTYPE Select() override { return perform(VisibleListAction::Select); }
    HRESULT STDMETHODCALLTYPE AddToSelection() override { return perform(VisibleListAction::Add); }
    HRESULT STDMETHODCALLTYPE RemoveFromSelection() override { return perform(VisibleListAction::Remove); }
    HRESULT STDMETHODCALLTYPE get_IsSelected(BOOL *out) override
    {
        if (!out) return E_POINTER;
        if (!stable()) return UIA_E_ELEMENTNOTAVAILABLE;
        *out = (::SendMessage(hwnd, LVM_GETITEMSTATE, row, LVIS_SELECTED) & LVIS_SELECTED) != 0;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_SelectionContainer(IRawElementProviderSimple **out) override
    {
        if (!out) return E_POINTER;
        root->AddRef(); *out = static_cast<IRawElementProviderSimple *>(root); return S_OK;
    }
    HRESULT STDMETHODCALLTYPE Invoke() override { return perform(VisibleListAction::Invoke); }
    HRESULT STDMETHODCALLTYPE Realize() override
    {
        if (!valid() || row < 0 || row >= count()) return UIA_E_ELEMENTNOTAVAILABLE;
        ::SendMessage(hwnd, LVM_ENSUREVISIBLE, row, FALSE);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE Scroll(ScrollAmount horizontal, ScrollAmount vertical) override
    {
        if (!valid()) return UIA_E_ELEMENTNOTAVAILABLE;
        if (horizontal != ScrollAmount_NoAmount) return UIA_E_INVALIDOPERATION;
        int change = 0;
        if (vertical == ScrollAmount_SmallIncrement) change = 1;
        else if (vertical == ScrollAmount_SmallDecrement) change = -1;
        else if (vertical == ScrollAmount_LargeIncrement) change = page() - 1;
        else if (vertical == ScrollAmount_LargeDecrement) change = -(page() - 1);
        else if (vertical != ScrollAmount_NoAmount) return E_INVALIDARG;
        if (count() && change) ::SendMessage(hwnd, LVM_ENSUREVISIBLE, (std::max)(0, (std::min)(count() - 1, change > 0 ? last() + change : first() + change)), FALSE);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetScrollPercent(double horizontal, double vertical) override
    {
        if (!valid()) return UIA_E_ELEMENTNOTAVAILABLE;
        if (horizontal != UIA_ScrollPatternNoScroll || (vertical != UIA_ScrollPatternNoScroll && (vertical < 0 || vertical > 100))) return E_INVALIDARG;
        if (vertical != UIA_ScrollPatternNoScroll && count()) ::SendMessage(hwnd, LVM_ENSUREVISIBLE, static_cast<int>((count() - 1) * vertical / 100.0), FALSE);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_HorizontalScrollPercent(double *out) override { if (!out) return E_POINTER; *out = UIA_ScrollPatternNoScroll; return S_OK; }
    HRESULT STDMETHODCALLTYPE get_VerticalScrollPercent(double *out) override { if (!out) return E_POINTER; int max = (std::max)(0, count() - page()); *out = max ? 100.0 * first() / max : UIA_ScrollPatternNoScroll; return S_OK; }
    HRESULT STDMETHODCALLTYPE get_HorizontalViewSize(double *out) override { if (!out) return E_POINTER; *out = 100; return S_OK; }
    HRESULT STDMETHODCALLTYPE get_VerticalViewSize(double *out) override { if (!out) return E_POINTER; *out = count() ? (std::min)(100.0, 100.0 * page() / count()) : 100; return S_OK; }
    HRESULT STDMETHODCALLTYPE get_HorizontallyScrollable(BOOL *out) override { if (!out) return E_POINTER; *out = FALSE; return S_OK; }
    HRESULT STDMETHODCALLTYPE get_VerticallyScrollable(BOOL *out) override { if (!out) return E_POINTER; *out = count() > page(); return S_OK; }
};
