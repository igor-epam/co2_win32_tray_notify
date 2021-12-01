// CO2Tracker.cpp : Defines the entry point for the application.
//

#include "CO2Tracker.h"

#include "WinApiException.h"
// we need commctrl v6 for LoadIconMetric()
#pragma comment(linker, \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

#include <WinUser.h>
#include <commctrl.h>
#include <shellapi.h>
#include <strsafe.h>
#include <windows.h>
#include <windowsx.h>

#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>

#include "resource.h"

using namespace co2;

namespace {

UINT constexpr WMAPP_NOTIFYCALLBACK = WM_APP + 1;
class __declspec(uuid("1b1793c0-7522-4263-b290-af8ec052dc64")) CO2Icon;
auto constexpr ClassName = L"CO2Tracker";

void RegisterWindowClass(
    HINSTANCE h_instance, WNDPROC wnd_proc, std::wstring_view class_name) {
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = wnd_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = h_instance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = L"MainMenu";
    wc.lpszClassName = class_name.data();
    if (!::RegisterClass(&wc)) {
        throw WinApiException(L"Unable to create window.");
    }
}

void ShowContextMenu(HINSTANCE hinstance, HWND hwnd, POINT pt) {
    HMENU hMenu = LoadMenu(hinstance, MAKEINTRESOURCE(IDR_MENU1));
    if (hMenu) {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu) {
            // our window must be foreground before calling TrackPopupMenu or
            // the menu will not disappear when the user clicks away
            SetForegroundWindow(hwnd);

            // respect menu drop alignment
            UINT uFlags = TPM_RIGHTBUTTON;
            if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
                uFlags |= TPM_RIGHTALIGN;
            } else {
                uFlags |= TPM_LEFTALIGN;
            }

            TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}

}  // namespace

CO2Tracker::CO2Tracker(HINSTANCE instance)
    : instance_(instance),
      hwnd_(createMainWindow()),
      tray_icon_(instance_, hwnd_, __uuidof(CO2Icon), WMAPP_NOTIFYCALLBACK) {}

HWND CO2Tracker::createMainWindow() {
    RegisterWindowClass(instance_, &CO2Tracker::WindowProc, ClassName);
    auto const hwnd = CreateWindow(ClassName, L"", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 250, 200, NULL, NULL, instance_, NULL);
    if (!hwnd) {
        throw WinApiException(L"Unable to create main window.");
    }
    ::SetLastError(0);
    if (!::SetWindowLongPtr(
            hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this)) &&
        GetLastError()) {
        throw WinApiException(L"Unable to set this handle.");
    }
    return hwnd;
}

int APIENTRY wWinMain(
    HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /*nCmdShow*/) {
    try {
        CO2Tracker tracker(hInstance);
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } catch (std::exception const& ex) {
        MessageBoxW(NULL, L"Startup error",
            reinterpret_cast<wchar_t const*>(ex.what()), MB_OK);
    }
    return 0;
}

LRESULT CALLBACK CO2Tracker::WindowProc(
    HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    auto const tracker =
        reinterpret_cast<CO2Tracker*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (nullptr != tracker) {
        return tracker->windowProc(hwnd, msg, wparam, lparam);
    } else {
        return ::DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

LRESULT CALLBACK CO2Tracker::windowProc(
    HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_COMMAND: {
            auto const wmId = LOWORD(wparam);
            auto const wmEvent = HIWORD(wparam);
            // Parse the menu selections:

            switch (wmId) {
                case ID__EXIT:
                    DestroyWindow(hwnd);
                    break;
                default:
                    return ::DefWindowProc(hwnd, msg, wparam, lparam);
            }
            break;
        }
        case WMAPP_NOTIFYCALLBACK: {
            switch (LOWORD(lparam)) {
                case WM_CONTEXTMENU: {
                    POINT const pt = {LOWORD(wparam), HIWORD(wparam)};
                    ShowContextMenu(instance_, hwnd, pt);
                } break;
            }
            break;
        }
        case WM_DESTROY:
            ::PostQuitMessage(0);
            break;
        default:
            return ::DefWindowProc(hwnd, msg, wparam, lparam);
    }
    return 0;
}