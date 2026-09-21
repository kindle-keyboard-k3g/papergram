#include "session_storage.h"

#include <cerrno>
#include <cstdio>
#include <fstream>
#include <sys/stat.h>

namespace mtproto {
namespace {

const std::uint32_t magic = 0x534d4754U;
const std::uint32_t version = 1U;

void write_u32(std::ostream& output, std::uint32_t value) {
    for (unsigned index = 0U; index < 4U; ++index) {
        output.put(static_cast<char>(value >> (index * 8U)));
    }
}

void write_u64(std::ostream& output, std::uint64_t value) {
    for (unsigned index = 0U; index < 8U; ++index) {
        output.put(static_cast<char>(value >> (index * 8U)));
    }
}

bool read_u32(std::istream& input, std::uint32_t& value) {
    value = 0U;
    for (unsigned index = 0U; index < 4U; ++index) {
        const int byte = input.get();
        if (byte == EOF) {
            return false;
        }
        value |= static_cast<std::uint32_t>(static_cast<unsigned char>(byte)) << (index * 8U);
    }
    return true;
}

bool read_u64(std::istream& input, std::uint64_t& value) {
    value = 0U;
    for (unsigned index = 0U; index < 8U; ++index) {
        const int byte = input.get();
        if (byte == EOF) {
            return false;
        }
        value |= static_cast<std::uint64_t>(static_cast<unsigned char>(byte)) << (index * 8U);
    }
    return true;
}

bool make_directory(const std::string& path) {
    if (path.empty() || path == ".") {
        return true;
    }
    const std::size_t separator = path.find_last_of('/');
    if (separator != std::string::npos && !make_directory(path.substr(0U, separator))) {
        return false;
    }
    if (::mkdir(path.c_str(), 0700) == 0 || errno == EEXIST) {
        return true;
    }
    return false;
}

bool create_parent(const std::string& path) {
    const std::size_t separator = path.find_last_of('/');
    if (separator == std::string::npos) {
        return true;
    }
    return make_directory(path.substr(0U, separator));
}

}

SessionStorage::SessionStorage(const std::string& path) : path_(path) {}

bool SessionStorage::save(const SessionData& session) const {
    if (session.auth_key.empty() || session.auth_key.size() > 4096U || !create_parent(path_)) {
        return false;
    }
    const std::string temporary = path_ + ".tmp";
    std::ofstream output(temporary.c_str(), std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }
    write_u32(output, magic);
    write_u32(output, version);
    write_u32(output, static_cast<std::uint32_t>(session.auth_key.size()));
    output.write(reinterpret_cast<const char*>(session.auth_key.data()),
                 static_cast<std::streamsize>(session.auth_key.size()));
    write_u64(output, session.server_salt);
    write_u64(output, session.session_id);
    write_u32(output, static_cast<std::uint32_t>(session.dc_id));
    output.close();
    if (!output) {
        std::remove(temporary.c_str());
        return false;
    }
    if (std::rename(temporary.c_str(), path_.c_str()) != 0) {
        std::remove(temporary.c_str());
        return false;
    }
    return true;
}

bool SessionStorage::load(SessionData& session) const {
    std::ifstream input(path_.c_str(), std::ios::binary);
    if (!input) {
        return false;
    }
    std::uint32_t file_magic = 0U;
    std::uint32_t file_version = 0U;
    std::uint32_t key_size = 0U;
    if (!read_u32(input, file_magic) || !read_u32(input, file_version) || !read_u32(input, key_size)) {
        return false;
    }
    if (file_magic != magic || file_version != version || key_size == 0U || key_size > 4096U) {
        return false;
    }
    SessionData result;
    result.auth_key.resize(key_size);
    input.read(reinterpret_cast<char*>(result.auth_key.data()), static_cast<std::streamsize>(key_size));
    if (input.gcount() != static_cast<std::streamsize>(key_size) ||
        !read_u64(input, result.server_salt) || !read_u64(input, result.session_id)) {
        return false;
    }
    std::uint32_t dc_id = 0U;
    if (!read_u32(input, dc_id)) {
        return false;
    }
    result.dc_id = static_cast<std::int32_t>(dc_id);
    session = result;
    return true;
}

bool SessionStorage::remove() const {
    return std::remove(path_.c_str()) == 0 || errno == ENOENT;
}

}
