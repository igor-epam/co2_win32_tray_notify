#include "TrayWindow.h"

#define _WINSOCKAPI_
#include <Windows.h>
#include <commctrl.h>

#include "StateTranslator.h"
#include "WinApiException.h"
#include "resource.h"

using namespace co2;

namespace {

std::atomic_bool is_first = true;
auto constexpr InitializingString = L"Initializing...";

void ShowNotificationIcon(HINSTANCE h_instance, HWND hwnd, GUID guid,
    UINT message_id, std::wstring_view text, LPWSTR icon) {
    NOTIFYICONDATA nid;
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.guidItem = guid;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
    ::LoadIconMetric(h_instance, icon, LIM_LARGE, &nid.hIcon);
    nid.uCallbackMessage = message_id;
    std::memcpy(nid.szTip, text.data(), text.size() * sizeof(WCHAR));
    Shell_NotifyIcon(is_first ? NIM_ADD : NIM_MODIFY, &nid);
    is_first = false;
    nid.uVersion = NOTIFYICON_VERSION_4;
    if (!Shell_NotifyIcon(NIM_SETVERSION, &nid)) {
        throw WinApiException(L"Unable to show tray icon.");
    }
    ::DestroyIcon(nid.hIcon);
}

DWORD ToBaloonWin32Slyte(BaloonStyle style) {
    switch (style) {
        case BaloonStyle::Info:
            return NIIF_USER;
        case BaloonStyle::Warning:
            return NIIF_WARNING;
        case BaloonStyle::Error:
            return NIIF_ERROR;
    }
    return NIIF_USER;
}


void ShowBaloon(HINSTANCE h_instance, GUID guid, std::wstring_view title,
    std::wstring_view message, LPWSTR icon, DWORD type) {
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.uFlags = NIF_INFO | NIF_GUID;
    nid.guidItem = guid;
    nid.dwInfoFlags = type | NIIF_LARGE_ICON;
    std::memcpy(nid.szInfoTitle, title.data(), title.size() * sizeof(WCHAR));
    std::memcpy(nid.szInfo, message.data(), message.size() * sizeof(WCHAR));
    //::LoadIconMetric(h_instance, icon, LIM_LARGE, &nid.hBalloonIcon);
    if (!Shell_NotifyIcon(NIM_MODIFY, &nid)) {
        throw WinApiException(L"Unable to show baloon.");
    }
    //::DestroyIcon(nid.hBalloonIcon);
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

TrayWindow::TrayWindow(HINSTANCE h_instance, HWND hwnd, GUID guid,
    UINT message_id, StateTranslator& translator)
    : translator_(translator),
      h_instance_(h_instance),
      hwnd_(hwnd),
      guid_(guid),
      message_id_(message_id),
      thread_([&]() {
          while (!end_) {
              std::unique_lock<std::mutex> lk(m_);
              cv_.wait(lk, [&] { return task_ || end_; });
              draw_new_state();
          }
      }) {
    auto default_state = translator_.ConnectionLost();
    UpdateIcon(default_state.style_, std::nullopt, default_state.text_);
}

TrayWindow::~TrayWindow() {
    end_ = true;
    cv_.notify_one();
    thread_.join();
    DeleteNotificationIcon(guid_);
}

void TrayWindow::Redraw() {
    std::lock_guard lock(state_mutex_);
    if (state_) {
        auto state_without_baloon =
            State{state_->tray_, std::nullopt, state_->text_};
        post_task(std::move(state_without_baloon));
    }
}

void TrayWindow::UpdateIcon(TrayIconStyle style,
    std::optional<BaloonState> ballon, std::wstring_view text) {
    post_task({style, ballon, std::wstring(text)});
}

void TrayWindow::post_task(State new_state) {
    std::lock_guard<std::mutex> lk(m_);
    task_ = std::move(new_state);
    cv_.notify_one();
}

void TrayWindow::draw_new_state() {
    if (task_) {
        std::lock_guard lock(state_mutex_);
        auto const icon_id = StyleToId(task_->tray_);
        ShowNotificationIcon(
            h_instance_, hwnd_, guid_, message_id_, task_->text_, icon_id);
        if (task_->baloon_) {
            auto const ballon_win32_style =
                ToBaloonWin32Slyte(task_->baloon_->style_);
            ShowBaloon(h_instance_, guid_, L"", task_->baloon_->message_,
                icon_id, ballon_win32_style);
        }
        state_ = std::move(task_);
        task_.reset();
    }
}