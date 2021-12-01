#pragma once

#include <exception>
#include <string>
#include <string_view>

namespace co2 {

class WinApiException : public std::exception {
   public:
    WinApiException(std::wstring_view message)
        : std::exception(reinterpret_cast<char const*>(
              (std::wstring(message) + L". WinApi code: " +
                  std::to_wstring(::GetLastError()))
                  .c_str())) {}
};

}  // namespace co2