#ifndef KINDLE_MTPROTO_TL_CODEC_H
#define KINDLE_MTPROTO_TL_CODEC_H

#include <cstdint>
#include <string>
#include <vector>

namespace mtproto {

class TlWriter {
public:
    void write_int32(std::int32_t value);
    void write_int64(std::int64_t value);
    void write_string(const std::string& value);
    void write_bytes(const std::vector<std::uint8_t>& value);
    void write_int32_vector(const std::vector<std::int32_t>& value);
    const std::vector<std::uint8_t>& data() const;

private:
    std::vector<std::uint8_t> data_;
};

class TlReader {
public:
    explicit TlReader(const std::vector<std::uint8_t>& data);

    std::int32_t read_int32();
    std::int64_t read_int64();
    std::string read_string();
    std::vector<std::uint8_t> read_bytes();
    std::vector<std::int32_t> read_int32_vector();
    bool empty() const;

private:
    std::vector<std::uint8_t> data_;
    std::size_t offset_;

    void require(std::size_t size) const;
    std::vector<std::uint8_t> read_tl_bytes();
};

}

#endif
