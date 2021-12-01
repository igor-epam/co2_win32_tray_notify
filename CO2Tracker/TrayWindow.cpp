#include "TrayWindow.h"

#include <Windows.h>
#include <commctrl.h>

#include "WinApiException.h"
#include "resource.h"

using namespace co2;

namespace {

auto constexpr InitializingString = L"Initializing...";

void ShowNotificationIcon(HINSTANCE h_instance, HWND hwnd, GUID guid,
    UINT message_id, std::wstring_view text, LPWSTR icon) {
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.hWnd = hwnd;
    nid.guidItem = guid;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
    ::LoadIconMetric(h_instance, icon, LIM_SMALL, &nid.hIcon);
    nid.uCallbackMessage = message_id;
    std::memcpy(nid.szTip, text.data(), text.size() * sizeof(WCHAR));
    Shell_NotifyIcon(NIM_ADD, &nid);
    nid.uVersion = NOTIFYICON_VERSION_4;
    if (!Shell_NotifyIcon(NIM_SETVERSION, &nid)) {
        throw WinApiException(L"Unable to show tray icon.");
    }
}

LPWSTR StyleToId(TrayIconStyle style) {
    switch (style) {
        case TrayIconStyle::Green:
            return MAKEINTRESOURCE(IDI_GREEN);
        case TrayIconStyle::Red:
            return MAKEINTRESOURCE(IDI_RED);
        case TrayIconStyle::Undefined:
            return MAKEINTRESOURCE(IDI_GREY);
        case TrayIconStyle::Yellow:
            return MAKEINTRESOURCE(IDI_YELLOW);
        case TrayIconStyle::Black:
            return MAKEINTRESOURCE(IDI_BLACK);
        default:
            throw std::exception("not yet implemented.");
    }
}

BOOL DeleteNotificationIcon(GUID guid) {
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.uFlags = NIF_GUID;
    nid.guidItem = guid;
    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

}  // namespace

TrayWindow::TrayWindow(
    HINSTANCE h_instance, HWND hwnd, GUID guid, UINT message_id)
    : h_instance_(h_instance),
      hwnd_(hwnd),
      guid_(guid),
      message_id_(message_id) {
    ShowNotificationIcon(h_instance_, hwnd_, guid_, message_id_,
        InitializingString, StyleToId(TrayIconStyle::Undefined));
}

TrayWindow::~TrayWindow() { DeleteNotificationIcon(guid_); }

void TrayWindow::UpdateIcon(TrayIconStyle style, std::wstring_view text) {
    ShowNotificationIcon(
        h_instance_, hwnd_, guid_, message_id_, text, StyleToId(style));
}
