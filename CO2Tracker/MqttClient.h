#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <functional>
#include <memory>
#include <mqtt/strand.hpp>
#include <string>
#include <thread>

namespace mqtt {
template <typename Impl>
struct callable_overlay;

template <typename Socket, std::size_t PacketIdBytes>
class sync_client;

template <typename Socket, typename Strand>
class tcp_endpoint;

}  // namespace mqtt

namespace co2 {

class MqttClient final {
   public:
    using OnUpdate = std::function<void(double)>;
    using OnConnect = std::function<void(void)>;
    using OnDisconnect = std::function<void(bool)>;

    MqttClient(std::wstring host, int port, std::wstring user,
        std::wstring password, std::wstring state_topic,
        std::wstring query_topic, std::wstring will_topic, OnUpdate on_update, OnConnect on_connect,
        OnDisconnect on_disconnect);
    ~MqttClient();

    void ForceQuery();

    void Stop();

   private:
    void disconnected();

    void on_publish(std::string_view topic, std::string_view data);

    OnDisconnect on_disconnect_;
    OnConnect on_connect_;
    OnUpdate on_update_;
    std::string state_topic_;
    std::string query_topic_;
    std::string will_topic_;

    boost::asio::io_context ioc_;
    std::shared_ptr<mqtt::callable_overlay<mqtt::sync_client<
        mqtt::tcp_endpoint<boost::asio::ip::tcp::socket, mqtt::strand>, 2> > >
        client_;
    std::thread thread_;
};

}  // namespace co2