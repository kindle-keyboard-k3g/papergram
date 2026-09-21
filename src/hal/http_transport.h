#ifndef KINDLE_HAL_HTTP_TRANSPORT_H
#define KINDLE_HAL_HTTP_TRANSPORT_H

#include "network_transport.h"

#include <cstdint>
#include <string>

struct HttpProxy {
    std::string host;
    std::uint16_t port;

    HttpProxy();
    HttpProxy(const std::string& proxy_host, std::uint16_t proxy_port);
    bool enabled() const;
};

class HttpTransport : public INetworkTransport {
public:
    HttpTransport(const std::string& host, std::uint16_t port,
                  const HttpProxy& proxy = HttpProxy());
    bool post(const std::string& url,
              const std::vector<std::uint8_t>& payload,
              std::vector<std::uint8_t>& response) override;

private:
    struct Endpoint {
        std::string host;
        std::uint16_t port;
    };

    Endpoint endpoint_;
    HttpProxy proxy_;

    int connect_socket() const;
    bool establish_proxy(int socket) const;
    bool send_all(int socket, const std::string& data) const;
    bool receive_response(int socket, std::vector<std::uint8_t>& response) const;
};

#endif
