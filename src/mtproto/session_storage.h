#ifndef KINDLE_MTPROTO_SESSION_STORAGE_H
#define KINDLE_MTPROTO_SESSION_STORAGE_H

#include <cstdint>
#include <string>
#include <vector>

namespace mtproto {

struct SessionData {
    std::vector<std::uint8_t> auth_key;
    std::uint64_t server_salt;
    std::uint64_t session_id;
    std::int32_t dc_id;
};

using Session = SessionData;

class SessionStorage {
public:
    explicit SessionStorage(const std::string& path = "/mnt/us/telegram/session.dat");

    bool save(const SessionData& session) const;
    bool load(SessionData& session) const;
    bool remove() const;

private:
    std::string path_;
};

}

#endif
