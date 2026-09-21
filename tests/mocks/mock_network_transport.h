#ifndef KINDLE_TESTS_MOCKS_MOCK_NETWORK_TRANSPORT_H
#define KINDLE_TESTS_MOCKS_MOCK_NETWORK_TRANSPORT_H

#include "../../src/hal/network_transport.h"
#include <string>
#include <vector>

class MockNetworkTransport : public INetworkTransport {
public:
    void setResponse(const std::vector<std::uint8_t>& response) {
        canned_response_ = response;
    }

    bool post(const std::string& url,
              const std::vector<std::uint8_t>& payload,
              std::vector<std::uint8_t>& response) override {
        last_url_ = url;
        last_payload_ = payload;
        response = canned_response_;
        call_count_++;
        return success_;
    }

    void setSuccess(bool success) {
        success_ = success;
    }

    int callCount() const { return call_count_; }
    const std::string& lastUrl() const { return last_url_; }
    const std::vector<std::uint8_t>& lastPayload() const { return last_payload_; }

private:
    std::string last_url_;
    std::vector<std::uint8_t> last_payload_;
    std::vector<std::uint8_t> canned_response_;
    int call_count_ = 0;
    bool success_ = true;
};

#endif
