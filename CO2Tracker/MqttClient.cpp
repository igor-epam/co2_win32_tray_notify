#include "MqttClient.h"

#include <charconv>
#include <mqtt/sync_client.hpp>

#include "Strings.h"

using namespace co2;

MqttClient::MqttClient(std::wstring host, int port, std::wstring user,
    std::wstring password, std::wstring state_topic, std::wstring query_topic,
    std::wstring will_topic, OnUpdate on_update, OnConnect on_connect,
    OnDisconnect on_disconnect)
    : on_disconnect_(std::move(on_disconnect)),
      on_connect_(std::move(on_connect)),
      on_update_(std::move(on_update)),
      state_topic_(ws2s(state_topic)),
      query_topic_(ws2s(query_topic)),
      will_topic_(ws2s(will_topic)),
      ioc_(),
      client_([&]() {
          auto client =
              MQTT_NS::make_sync_client(ioc_, ws2s(host), std::to_string(port));
          client->set_user_name(ws2s(user));
          client->set_password(ws2s(password));
          client->set_client_id("co2Tracker");
          client->set_clean_session(true);
          client->set_error_handler(
              [&](MQTT_NS::error_code) { disconnected(); });
          client->set_disconnect_handler([&]() { disconnected(); });
          client->set_publish_handler(
              [&](boost::optional<
                      std::remove_reference_t<decltype(*client)>::packet_id_t>
                      packet_id,
                  MQTT_NS::publish_options pubopts, MQTT_NS::buffer topic_name,
                  MQTT_NS::buffer contents) {
                  on_publish({topic_name.data(), topic_name.size()},
                      {contents.data(), contents.size()});
                  return true;
              });
          client->set_connack_handler(
              [&, state_t = ws2s(state_topic), will_t = ws2s(will_topic)](
                  bool sp,
                  MQTT_NS::connect_return_code connack_return_code) mutable {
                  if (connack_return_code ==
                      MQTT_NS::connect_return_code::accepted) {
                      on_connect_();
                      client_->subscribe(
                          std::move(state_t), MQTT_NS::qos::at_most_once);
                      client_->subscribe(
                          std::move(will_t), MQTT_NS::qos::at_most_once);
                  }
                  return true;
              });
          client->connect();

          return client;
      }()),
      thread_([&]() mutable {
          try {
              ioc_.run();
          } catch (std::exception const& /*ex*/) {

          }
      }) {}

MqttClient::~MqttClient() {
    try {
        Stop();
    } catch (std::exception const& /*ex*/) {
    }
}

void MqttClient::Stop() {
    if (client_) {
        client_->disconnect();
        ioc_.stop();
        on_disconnect_(true);
        client_.reset();
    }
    if (thread_.joinable()) thread_.join();
}

void MqttClient::ForceQuery() {
    client_->publish(query_topic_, "", MQTT_NS::qos::at_most_once);
}

void MqttClient::disconnected() { on_disconnect_(false); }

void MqttClient::on_publish(std::string_view topic, std::string_view data) {
    if (topic == state_topic_) {
        double value = 0;
        auto [ptr, ec]{
            std::from_chars(data.data(), data.data() + data.size(), value)};
        if (ec == std::errc()) {
            on_update_(value);
        }
    } else if (topic == will_topic_) {
        if (data == "on") {
            on_connect_();
        } else if (data == "off") {
            disconnected();
        }
    }
}