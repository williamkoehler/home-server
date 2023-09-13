#pragma once
#include <common/common.hpp>

#include <boost/beast.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            namespace http
            {
                template <class Caller>
                class HttpClient
                {
                  private:
                    boost::beast::tcp_stream stream;
                    boost::beast::flat_buffer buffer;

                    // boost::beast::http::request<boost::beast::string>

                    void OnResolve();

                    void OnConnect();
                    void OnWrite();
                    void OnRead();

                  private:
                };
            }
        }
    }
}