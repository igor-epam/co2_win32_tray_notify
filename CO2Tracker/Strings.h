#pragma once

#include <codecvt>
#include <string>

namespace co2 {

inline std::wstring s2ws(const std::string& str) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converter;

    return converter.from_bytes(str);
}

inline std::string ws2s(const std::wstring& wstr) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converter;

    return converter.to_bytes(wstr);
}

}  // namespace co2