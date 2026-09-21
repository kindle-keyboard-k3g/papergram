#include "http_transport.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace {

const int socket_timeout_seconds = 20;

void configure_timeout(int socket) {
    timeval timeout{};
    timeout.tv_sec = socket_timeout_seconds;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}

std::string request_path(const std::string& url) {
    const std::size_t scheme = url.find("://");
    const std::size_t authority = scheme == std::string::npos ? 0U : scheme + 3U;
    const std::size_t slash = url.find('/', authority);
    return slash == std::string::npos ? "/" : url.substr(slash);
}

bool write_bytes(int socket, const char* data, std::size_t size) {
    while (size != 0U) {
        const ssize_t written = send(socket, data, size, 0);
        if (written <= 0) {
            return false;
        }
        data += written;
        size -= static_cast<std::size_t>(written);
    }
    return true;
}

bool status_is_success(const std::string& headers) {
    const std::size_t line_end = headers.find("\r\n");
    const std::string status = headers.substr(0U, line_end);
    return status.find(" 2") != std::string::npos;
}

std::size_t body_length(const std::string& headers) {
    std::string lower(headers);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char value) {
        return static_cast<char>(std::tolower(value));
    });
    const std::string marker = "content-length:";
    const std::size_t start = lower.find(marker);
    if (start == std::string::npos) {
        return 0U;
    }
    const std::size_t value_start = lower.find_first_not_of(' ', start + marker.size());
    return value_start == std::string::npos ? 0U : static_cast<std::size_t>(std::stoul(lower.substr(value_start)));
}

}

HttpProxy::HttpProxy() : host(), port(0U) {}

HttpProxy::HttpProxy(const std::string& proxy_host, std::uint16_t proxy_port)
    : host(proxy_host), port(proxy_port) {}

bool HttpProxy::enabled() const {
    return !host.empty() && port != 0U;
}

HttpTransport::HttpTransport(const std::string& host, std::uint16_t port, const HttpProxy& proxy)
    : endpoint_{host, port}, proxy_(proxy) {}

int HttpTransport::connect_socket() const {
    const std::string& host = proxy_.enabled() ? proxy_.host : endpoint_.host;
    const std::uint16_t port = proxy_.enabled() ? proxy_.port : endpoint_.port;
    addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    addrinfo* addresses = nullptr;
    const std::string service = std::to_string(port);
    if (getaddrinfo(host.c_str(), service.c_str(), &hints, &addresses) != 0) {
        return -1;
    }
    int socket = -1;
    for (addrinfo* address = addresses; address != nullptr; address = address->ai_next) {
        socket = ::socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (socket < 0) {
            continue;
        }
        configure_timeout(socket);
        if (::connect(socket, address->ai_addr, address->ai_addrlen) == 0) {
            break;
        }
        ::close(socket);
        socket = -1;
    }
    freeaddrinfo(addresses);
    return socket;
}

bool HttpTransport::send_all(int socket, const std::string& data) const {
    return write_bytes(socket, data.data(), data.size());
}

bool HttpTransport::establish_proxy(int socket) const {
    if (!proxy_.enabled()) {
        return true;
    }
    std::ostringstream request;
    request << "CONNECT " << endpoint_.host << ":" << endpoint_.port << " HTTP/1.1\r\n"
            << "Host: " << endpoint_.host << ":" << endpoint_.port << "\r\n"
            << "Connection: keep-alive\r\n\r\n";
    if (!send_all(socket, request.str())) {
        return false;
    }
    std::string headers;
    char byte = 0;
    while (headers.find("\r\n\r\n") == std::string::npos && headers.size() < 8192U) {
        if (recv(socket, &byte, 1U, 0) != 1) {
            return false;
        }
        headers.push_back(byte);
    }
    return status_is_success(headers);
}

bool HttpTransport::receive_response(int socket, std::vector<std::uint8_t>& response) const {
    std::string received;
    char buffer[4096];
    ssize_t count = 0;
    while ((count = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        received.append(buffer, static_cast<std::size_t>(count));
    }
    const std::size_t separator = received.find("\r\n\r\n");
    if (separator == std::string::npos || !status_is_success(received.substr(0U, separator))) {
        return false;
    }
    const std::size_t body_start = separator + 4U;
    const std::size_t declared_length = body_length(received.substr(0U, separator));
    const std::size_t available = received.size() - body_start;
    const std::size_t length = declared_length == 0U ? available : std::min(declared_length, available);
    response.assign(received.begin() + static_cast<std::ptrdiff_t>(body_start),
                    received.begin() + static_cast<std::ptrdiff_t>(body_start + length));
    return true;
}

bool HttpTransport::post(const std::string& url,
                         const std::vector<std::uint8_t>& payload,
                         std::vector<std::uint8_t>& response) {
    response.clear();
    const int socket = connect_socket();
    if (socket < 0 || !establish_proxy(socket)) {
        if (socket >= 0) {
            ::close(socket);
        }
        return false;
    }
    std::ostringstream header;
    header << "POST " << request_path(url) << " HTTP/1.1\r\n"
           << "Host: " << endpoint_.host << "\r\n"
           << "Content-Type: application/octet-stream\r\n"
           << "Content-Length: " << payload.size() << "\r\n"
           << "Connection: close\r\n\r\n";
    const bool sent = send_all(socket, header.str()) &&
        write_bytes(socket, reinterpret_cast<const char*>(payload.data()), payload.size());
    const bool received = sent && receive_response(socket, response);
    ::close(socket);
    return received;
}
