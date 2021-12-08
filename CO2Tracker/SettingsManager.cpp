#include "SettingsManager.h"

#include <boost/program_options/detail/convert.hpp>
#include <boost/program_options/detail/utf8_codecvt_facet.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace co2;

Settings SettingsManager::GetSettings() {
    const std::locale userLocale("");
    std::wifstream iniStream("settings.ini");
    iniStream.imbue(userLocale);
    boost::property_tree::wptree iniWPTree;
    boost::property_tree::ini_parser::read_ini(iniStream, iniWPTree);
    auto broker = iniWPTree.get<std::wstring>(L"mqtt.broker", L"");
    auto port = iniWPTree.get<int>(L"mqtt.port", 1883);
    auto update_topic = iniWPTree.get<std::wstring>(L"mqtt.update_topic", L"");
    auto query_topic = iniWPTree.get<std::wstring>(L"mqtt.query_topic", L"");
    auto will_topic = iniWPTree.get<std::wstring>(L"mqtt.will_topic", L"");
    return {std::move(broker), port, std::move(update_topic),
        std::move(query_topic), std::move(will_topic)};
}