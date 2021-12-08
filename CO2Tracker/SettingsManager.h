#pragma once

#include <string>

namespace co2 {

    struct Settings {
        std::wstring mqtt_broker_;
        int port_;
        std::wstring state_topic_;
        std::wstring query_topic_;
        std::wstring will_topic_;
    };


class SettingsManager final {
   public:

       SettingsManager() = delete;

       static Settings GetSettings();

   private:
};

}  // namespace co2