#ifndef KINDLE_HAL_NETWORK_TRANSPORT_H
#define KINDLE_HAL_NETWORK_TRANSPORT_H

#include <cstdint>
#include <string>
#include <vector>

class INetworkTransport {
public:
    virtual ~INetworkTransport() = default;

    virtual bool post(const std::string& url,
                      const std::vector<std::uint8_t>& payload,
                      std::vector<std::uint8_t>& response) = 0;
};

#endif
