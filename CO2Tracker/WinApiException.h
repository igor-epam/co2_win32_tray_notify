#pragma once

#include <exception>
#include <string>
#include <string_view>

#include "Strings.h"

#define _WINSOCKAPI_
#include <Windows.h>

namespace co2 {

class WinApiException : public std::exception {
   public:
    WinApiException(std::wstring_view message)
        : std::exception(ws2s((std::wstring(message) + L". WinApi code: " +
                                  std::to_wstring(::GetLastError())))
                             .c_str()) {}
};

}  // namespace co2