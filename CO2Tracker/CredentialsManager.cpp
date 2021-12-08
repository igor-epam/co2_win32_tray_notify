#include "CredentialsManager.h"

#include <sstream>
#include <vector>

#include "WinApiException.h"

#define _WINSOCKAPI_
#include <Windows.h>
#include <wincred.h>

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Credui.lib")

using namespace co2;

namespace {

class PCredentialGuard final {
    PCREDENTIALW p_credential_;

   public:
    PCredentialGuard() = default;
    ~PCredentialGuard() { ::CredFree(p_credential_); }

    PCREDENTIALW* GetPCredential() { return &p_credential_; }
};
}  // namespace

CredentialManager& CredentialManager::GetInstance() {
    static CredentialManager instance;
    return instance;
}

std::optional<AuthInfo> CredentialManager::GetAuthInfo(
    std::wstring id, std::wstring readable_id_name) {
    PCredentialGuard p_cred_guard;
    auto p_cred = p_cred_guard.GetPCredential();
    auto const exist = ::CredRead(id.data(), CRED_TYPE_GENERIC, NULL, p_cred);
    if (exist) {
        return {
            { std::move(id), std::wstring((*p_cred)->UserName), std::wstring(reinterpret_cast<wchar_t const*>((*p_cred)->CredentialBlob), (*p_cred)->CredentialBlobSize)
            }
        };
    } else {
        auto const message_text =
            std::wstring(L"Enter credentials for MQTT broker ") +
            readable_id_name;

        CREDUI_INFO cui_info;
        cui_info.cbSize = sizeof(CREDUI_INFO);
        cui_info.hbmBanner = nullptr;
        cui_info.hwndParent = nullptr;
        cui_info.pszCaptionText = id.data();
        cui_info.pszMessageText = message_text.data();

        DWORD dw_auth_error = 0;
        ULONG dw_auth_package = 0;

        LPVOID out_cred_buffer = nullptr;
        ULONG out_cred_buffer_size = 0;
        BOOL cred_save_checkbox = true;

        DWORD dwError = 0;

        dwError = CredUIPromptForWindowsCredentials(&cui_info, dw_auth_error,
            &dw_auth_package, nullptr, NULL, &out_cred_buffer,
            &out_cred_buffer_size, &cred_save_checkbox,
            CREDUIWIN_CHECKBOX | CREDUIWIN_GENERIC);

        if (dwError == ERROR_SUCCESS) {
            DWORD user_name_size = CREDUI_MAX_USERNAME_LENGTH;
            DWORD domain_name_size = CREDUI_MAX_DOMAIN_TARGET_LENGTH;
            DWORD password_size = CREDUI_MAX_PASSWORD_LENGTH;

            std::vector<WCHAR> user_name(user_name_size, 0);
            std::vector<WCHAR> domain(domain_name_size, 0);
            std::vector<WCHAR> password(password_size, 0);
            DWORD dwCredBufferSize = out_cred_buffer_size;  // ULONG to DWORD

            dwError =
                CredUnPackAuthenticationBuffer(CRED_PACK_GENERIC_CREDENTIALS,
                    out_cred_buffer, dwCredBufferSize, user_name.data(),
                    &user_name_size, domain.data(), &domain_name_size,
                    password.data(), &password_size);
            auto const last_error = GetLastError();

            // Check for error
            if (dwError != FALSE) {
                if (cred_save_checkbox) {
                    CREDENTIALW cred = {0};
                    cred.Type = CRED_TYPE_GENERIC;
                    cred.TargetName = id.data();
                    cred.CredentialBlobSize =
                        static_cast<DWORD>((password_size + 1) * sizeof(wchar_t));
                    cred.CredentialBlob =
                        reinterpret_cast<LPBYTE>(password.data());
                    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
                    cred.UserName = user_name.data();

                    CredWrite(&cred, 0);
                }
                SecureZeroMemory(out_cred_buffer, out_cred_buffer_size);
                CoTaskMemFree(out_cred_buffer);
                AuthInfo info{std::move(id),
                    std::wstring(user_name.data()),
                    std::wstring(password.data(), password_size)};
                return std::move(info);
            }
        }
    }

    return {};
}