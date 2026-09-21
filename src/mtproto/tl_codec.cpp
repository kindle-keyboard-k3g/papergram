#include "tl_codec.h"

#include <stdexcept>

namespace mtproto {
namespace {

const std::uint32_t vector_constructor = 0x1cb5c415U;

void append_little_endian(std::vector<std::uint8_t>& output, std::uint64_t value, unsigned bytes) {
    for (unsigned index = 0; index < bytes; ++index) {
        output.push_back(static_cast<std::uint8_t>(value >> (index * 8U)));
    }
}

std::uint32_t read_little_endian(const std::vector<std::uint8_t>& data, std::size_t offset, unsigned bytes) {
    std::uint32_t result = 0U;
    for (unsigned index = 0; index < bytes; ++index) {
        result |= static_cast<std::uint32_t>(data[offset + index]) << (index * 8U);
    }
    return result;
}

void append_tl_length(std::vector<std::uint8_t>& output, std::size_t size) {
    if (size < 254U) {
        output.push_back(static_cast<std::uint8_t>(size));
        return;
    }
    output.push_back(254U);
    append_little_endian(output, size, 3U);
}

void append_tl_bytes(std::vector<std::uint8_t>& output, const std::vector<std::uint8_t>& value) {
    const std::size_t size = value.size();
    if (size > 0xffffffU) {
        throw std::invalid_argument("TL byte string is too large");
    }
    append_tl_length(output, size);
    output.insert(output.end(), value.begin(), value.end());
    while ((output.size() % 4U) != 0U) {
        output.push_back(0U);
    }
}

}

void TlWriter::write_int32(std::int32_t value) {
    append_little_endian(data_, static_cast<std::uint32_t>(value), 4U);
}

void TlWriter::write_int64(std::int64_t value) {
    append_little_endian(data_, static_cast<std::uint64_t>(value), 8U);
}

void TlWriter::write_string(const std::string& value) {
    write_bytes(std::vector<std::uint8_t>(value.begin(), value.end()));
}

void TlWriter::write_bytes(const std::vector<std::uint8_t>& value) {
    append_tl_bytes(data_, value);
}

void TlWriter::write_int32_vector(const std::vector<std::int32_t>& value) {
    write_int32(static_cast<std::int32_t>(vector_constructor));
    write_int32(static_cast<std::int32_t>(value.size()));
    for (std::int32_t item : value) {
        write_int32(item);
    }
}

const std::vector<std::uint8_t>& TlWriter::data() const {
    return data_;
}

TlReader::TlReader(const std::vector<std::uint8_t>& data) : data_(data), offset_(0U) {}

void TlReader::require(std::size_t size) const {
    if (offset_ > data_.size() || size > data_.size() - offset_) {
        throw std::out_of_range("TL buffer is truncated");
    }
}

std::int32_t TlReader::read_int32() {
    require(4U);
    const std::int32_t result = static_cast<std::int32_t>(read_little_endian(data_, offset_, 4U));
    offset_ += 4U;
    return result;
}

std::int64_t TlReader::read_int64() {
    require(8U);
    std::uint64_t result = 0U;
    for (unsigned index = 0; index < 8U; ++index) {
        result |= static_cast<std::uint64_t>(data_[offset_ + index]) << (index * 8U);
    }
    offset_ += 8U;
    return static_cast<std::int64_t>(result);
}

std::vector<std::uint8_t> TlReader::read_tl_bytes() {
    require(1U);
    const std::uint8_t marker = data_[offset_++];
    std::size_t length = marker;
    std::size_t prefix = 1U;
    if (marker == 254U) {
        require(3U);
        length = read_little_endian(data_, offset_, 3U);
        offset_ += 3U;
        prefix = 4U;
    }
    require(length);
    std::vector<std::uint8_t> result(data_.begin() + offset_, data_.begin() + offset_ + length);
    offset_ += length;
    while (((prefix + length) % 4U) != 0U) {
        require(1U);
        ++offset_;
        ++prefix;
    }
    return result;
}

std::string TlReader::read_string() {
    const std::vector<std::uint8_t> value = read_tl_bytes();
    return std::string(value.begin(), value.end());
}

std::vector<std::uint8_t> TlReader::read_bytes() {
    return read_tl_bytes();
}

std::vector<std::int32_t> TlReader::read_int32_vector() {
    if (static_cast<std::uint32_t>(read_int32()) != vector_constructor) {
        throw std::invalid_argument("invalid TL vector constructor");
    }
    const std::int32_t count = read_int32();
    if (count < 0 || static_cast<std::size_t>(count) > (data_.size() - offset_) / 4U) {
        throw std::out_of_range("invalid TL vector length");
    }
    std::vector<std::int32_t> result;
    result.reserve(static_cast<std::size_t>(count));
    for (std::int32_t index = 0; index < count; ++index) {
        result.push_back(read_int32());
    }
    return result;
}

bool TlReader::empty() const {
    return offset_ == data_.size();
}

}
