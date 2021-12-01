#pragma once

#include <Windows.h>

#include <string_view>

namespace co2 {

enum class TrayIconStyle { Undefined, Green, Yellow, Red, Black };

class TrayWindow final {
   public:
    TrayWindow(HINSTANCE h_instance, HWND hwnd, GUID guid, UINT message_id);
    ~TrayWindow();

    TrayWindow(TrayWindow const&) = delete;
    TrayWindow& operator=(TrayWindow const&) = delete;

    void UpdateIcon(TrayIconStyle style, std::wstring_view text);

   private:
    HINSTANCE h_instance_;
    HWND hwnd_;
    GUID guid_;
    UINT message_id_;
};
}  // namespace co2