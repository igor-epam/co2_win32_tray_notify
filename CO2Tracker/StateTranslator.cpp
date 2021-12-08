#include "StateTranslator.h"

#include <sstream>

using namespace co2;

namespace {

double constexpr GreenMax = 600;
double constexpr YellowMax = 1000;
double constexpr RedMax = 1500;

template <int N = 1, typename T>
std::wstring to_string_with_precision(T a_value) {
    std::wostringstream out;
    out.precision(N);
    out << std::fixed << a_value;
    return out.str();
}

std::optional<BaloonState> ToBallonState(double new_value, double was_value) {
    if (was_value <= GreenMax && new_value > GreenMax)
        return BaloonState{BaloonStyle::Info,
            std::wstring(L"co2 level " +
                         to_string_with_precision<0>(new_value) +
                         L". Open the window.")};
    else if (was_value <= YellowMax && new_value > YellowMax)
        return BaloonState{BaloonStyle::Warning,
            std::wstring(L"co2 level " +
                         to_string_with_precision<0>(new_value) +
                         L". Open the window now.")};
    else if (was_value <= RedMax && new_value > RedMax)
        return BaloonState{BaloonStyle::Error,
            std::wstring(L"co2 level " +
                         to_string_with_precision<0>(new_value) +
                         L". Open the window ASAP.")};
    return {};
}
}  // namespace

TrayState StateTranslator::ToTrayState(double value) {
    struct JustBeforeDieSetNewValue {
        JustBeforeDieSetNewValue(double value, double& was_value)
            : new_value_(value), was_value_storage_(was_value) {}

        ~JustBeforeDieSetNewValue() { was_value_storage_ = new_value_; }

       private:
        double new_value_;
        double& was_value_storage_;
    } setter(value, was_value_);

    auto ballon_style = ToBallonState(value, was_value_);

    if (value < GreenMax) {
        return {to_string_with_precision(value), TrayIconStyle::Green,
            ballon_style};
    } else if (value < YellowMax) {
        return {to_string_with_precision(value), TrayIconStyle::Yellow,
            ballon_style};
    } else if (value < RedMax) {
        return {
            to_string_with_precision(value), TrayIconStyle::Red, ballon_style};
    } else {
        return {to_string_with_precision(value), TrayIconStyle::Black,
            ballon_style};
    }
}

TrayState StateTranslator::ConnectionLost() {
    return {L"no connection...", TrayIconStyle::Undefined,
        BaloonState{BaloonStyle::Error, L"no connection..."}};
}

TrayState StateTranslator::ConnectedWaitingForData() {
    return {L"connected, waiting for data", TrayIconStyle::Undefined, {}};
}

TrayState StateTranslator::NoCredentials() {
    return {L"not authenticated", TrayIconStyle::Undefined,
        BaloonState{BaloonStyle::Error, L"not authenticated"}};
}
