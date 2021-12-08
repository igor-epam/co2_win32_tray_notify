// CO2Tracker.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <Windows.h>

#include <memory>

#include "StateTranslator.h"
#include "TrayWindow.h"

namespace co2 {
class MqttClient;

class CO2Tracker final {
   public:
    CO2Tracker(HINSTANCE instance);
    ~CO2Tracker() = default;

    CO2Tracker(CO2Tracker const&) = delete;
    CO2Tracker& operator=(CO2Tracker const&) = delete;

    static LRESULT CALLBACK WindowProc(
        HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

   private:
    HWND createMainWindow();

    LRESULT CALLBACK windowProc(
        HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    StateTranslator translator_;
    HINSTANCE instance_;
    HWND hwnd_;
    TrayWindow tray_icon_;
    std::unique_ptr<MqttClient> _mqtt_client_;
};
}  // namespace co2