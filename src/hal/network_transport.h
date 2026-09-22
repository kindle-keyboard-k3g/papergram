#ifndef KINDLE_HAL_NETWORK_TRANSPORT_H
#define KINDLE_HAL_NETWORK_TRANSPORT_H

#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Abstract byte-oriented transport for HTTP-like POST requests.
 */
class INetworkTransport {
public:
    /** @brief Releases transport resources. */
    virtual ~INetworkTransport() = default;

    /**
     * @brief Sends a binary POST request and collects its binary response.
     * @param url Request URL or path used by the concrete transport.
     * @param payload Request body bytes.
     * @param response Destination for response body bytes.
     * @return true when the request and response completed successfully;
     *     otherwise false.
     */
    virtual bool post(const std::string& url,
                      const std::vector<std::uint8_t>& payload,
                      std::vector<std::uint8_t>& response) = 0;
};

#endif
