#ifndef KINDLE_MTPROTO_SESSION_STORAGE_H
#define KINDLE_MTPROTO_SESSION_STORAGE_H

#include <cstdint>
#include <string>
#include <vector>

namespace mtproto {

/**
 * @brief Persisted MTProto authorization and connection state.
 */
struct SessionData {
    /** @brief Authorization key bytes used for authenticated requests. */
    std::vector<std::uint8_t> auth_key;

    /** @brief Server salt associated with the authorization key. */
    std::uint64_t server_salt;

    /** @brief Session identifier used by MTProto messages. */
    std::uint64_t session_id;

    /** @brief Telegram data-center identifier for the session. */
    std::int32_t dc_id;
};

/** @brief Compatibility alias for persisted MTProto session data. */
using Session = SessionData;

/**
 * @brief Reads and writes authenticated MTProto session data on disk.
 */
class SessionStorage {
public:
    /**
     * @brief Creates storage backed by a session file.
     *
     * @param path File path used for saving, loading, and removing the session.
     */
    explicit SessionStorage(const std::string& path = "/mnt/us/telegram/session.dat");

    /**
     * @brief Atomically saves session data to the configured path.
     *
     * @param session Session state to persist.
     * @return `true` when the session is saved; `false` for invalid data or an
     *         filesystem failure.
     */
    bool save(const SessionData& session) const;

    /**
     * @brief Loads session data from the configured path.
     *
     * @param session Destination object populated on successful load.
     * @return `true` when a valid session is loaded; `false` when the file is
     *         missing, malformed, or unreadable.
     */
    bool load(SessionData& session) const;

    /**
     * @brief Removes the configured session file.
     *
     * @return `true` when the file is removed or was already absent; `false`
     *         when removal fails.
     */
    bool remove() const;

private:
    std::string path_;
};

}

#endif
