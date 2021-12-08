#pragma once

#include <string>
#include <optional>

#include "TrayWindow.h"

namespace co2 {

struct TrayState {
    std::wstring text_;
    TrayIconStyle style_;
    std::optional<BaloonState> baloon_;
};

class StateTranslator final {
   public:
    StateTranslator() = default;
    ~StateTranslator() = default;

    StateTranslator(StateTranslator&) = delete;
    StateTranslator& operator=(StateTranslator&) = delete;

    TrayState ToTrayState(double value);
    TrayState ConnectionLost();
    TrayState ConnectedWaitingForData();
    TrayState NoCredentials();

   private:
    double was_value_ = 0;
};

}  // namespace co2