#pragma once

#include <optional>
#include <string>

namespace co2 {

struct AuthInfo {
    std::wstring id_;
    std::wstring name_;
    std::wstring password_;
};

class CredentialManager final {
   public:
    std::optional<AuthInfo> GetAuthInfo(
        std::wstring id, std::wstring readable_id_name);

    static CredentialManager& GetInstance();

   private:
    CredentialManager() = default;
};

}  // namespace co2