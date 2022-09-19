#pragma once
#include "common.hpp"
#include <boost/array.hpp>
#include <home-common/Worker.hpp>

namespace server
{
    namespace networking
    {
        class BeaconListener : public boost::enable_shared_from_this<BeaconListener>
        {
          private:
            std::string nameCopy = "";
            std::string externalUrlCopy = "";

            Ref<udp_socket_t> listener;
            boost::array<char, 50> buffer;
            boost::asio::ip::udp::endpoint remoteEnpoint;

            void StartReceiving();
            void OnReceive(boost::system::error_code ec, size_t size);
            void OnSend(boost::system::error_code ec, size_t size, Ref<rapidjson::StringBuffer> message);

          public:
            BeaconListener(const std::string& externalURL);
            virtual ~BeaconListener();
            static Ref<BeaconListener> Create(const std::string& externalURL);
        };
    }
}