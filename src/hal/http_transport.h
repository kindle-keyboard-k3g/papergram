#ifndef KINDLE_HAL_HTTP_TRANSPORT_H
#define KINDLE_HAL_HTTP_TRANSPORT_H

#include "network_transport.h"

#include <cstdint>
#include <string>

/**
 * @brief Optional HTTP proxy endpoint used for outbound connections.
 */
struct HttpProxy {
    /** @brief Proxy hostname, or empty when no proxy is configured. */
    std::string host;

    /** @brief Proxy TCP port, or zero when no proxy is configured. */
    std::uint16_t port;

    /** @brief Constructs a disabled proxy configuration. */
    HttpProxy();

    /**
     * @brief Constructs a proxy configuration.
     * @param proxy_host Proxy hostname.
     * @param proxy_port Proxy TCP port.
     */
    HttpProxy(const std::string& proxy_host, std::uint16_t proxy_port);

    /**
     * @brief Reports whether this configuration enables proxy tunneling.
     * @return true when both host and port are configured; otherwise false.
     */
    bool enabled() const;
};

/**
 * @brief Minimal binary HTTP POST transport with optional CONNECT tunneling.
 *
 * Connections use the configured endpoint directly or establish an HTTP proxy
 * tunnel before sending the request.
 */
class HttpTransport : public INetworkTransport {
public:
    /**
     * @brief Creates a transport for a destination endpoint.
     * @param host Destination hostname.
     * @param port Destination TCP port.
     * @param proxy Optional HTTP proxy used to reach the destination.
     */
    HttpTransport(const std::string& host, std::uint16_t port,
                  const HttpProxy& proxy = HttpProxy());

    /**
     * @brief Sends a binary HTTP POST request.
     * @param url URL whose path is placed in the request line.
     * @param payload Request body bytes.
     * @param response Destination for the response body bytes; cleared first.
     * @return true when a successful HTTP response is received; otherwise
     *     false.
     */
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
