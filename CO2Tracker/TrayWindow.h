#pragma once

#define _WINSOCKAPI_
#include <Windows.h>
#include <winsock2.h>

#include <condition_variable>
#include <mutex>
#include <optional>
#include <string_view>
#include <thread>

namespace co2 {

enum class TrayIconStyle { Undefined, Green, Yellow, Red, Black };

enum class BaloonStyle { Info, Warning, Error };

struct BaloonState {
    BaloonStyle style_;
    std::wstring message_;
};

class StateTranslator;

class TrayWindow final {
   public:
    TrayWindow(HINSTANCE h_instance, HWND hwnd, GUID guid, UINT message_id,
        StateTranslator& translator);
    ~TrayWindow();

    TrayWindow(TrayWindow const&) = delete;
    TrayWindow& operator=(TrayWindow const&) = delete;

    void UpdateIcon(TrayIconStyle style, std::optional<BaloonState> ballon,
        std::wstring_view text);
    void Redraw();

   private:
    struct State {
        TrayIconStyle tray_;
        std::optional<BaloonState> baloon_;
        std::wstring text_;
    };

    void post_task(State new_state);
    void draw_new_state();

    StateTranslator& translator_;
    HINSTANCE h_instance_;
    HWND hwnd_;
    GUID guid_;
    UINT message_id_;
    std::thread thread_;
    std::mutex m_;
    std::condition_variable cv_;
    std::optional<State> task_ = std::nullopt;
    std::atomic<bool> end_ = false;

    std::mutex state_mutex_;
    std::optional<State> state_;
};
}  // namespace co2