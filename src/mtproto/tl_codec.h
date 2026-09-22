#ifndef KINDLE_MTPROTO_TL_CODEC_H
#define KINDLE_MTPROTO_TL_CODEC_H

#include <cstdint>
#include <string>
#include <vector>

namespace mtproto {

/**
 * @brief Serializes primitive values using Telegram's TL wire format.
 */
class TlWriter {
public:
    /**
     * @brief Appends a signed 32-bit integer in little-endian order.
     *
     * @param value Integer to append.
     */
    void write_int32(std::int32_t value);

    /**
     * @brief Appends a signed 64-bit integer in little-endian order.
     *
     * @param value Integer to append.
     */
    void write_int64(std::int64_t value);

    /**
     * @brief Appends a UTF-8 string as a TL byte string.
     *
     * @param value String to append.
     * @throws std::invalid_argument if the encoded string is larger than the
     *         maximum TL byte-string length.
     */
    void write_string(const std::string& value);

    /**
     * @brief Appends bytes with a TL length prefix and alignment padding.
     *
     * @param value Bytes to append.
     * @throws std::invalid_argument if the byte sequence is larger than the
     *         maximum TL byte-string length.
     */
    void write_bytes(const std::vector<std::uint8_t>& value);

    /**
     * @brief Appends a vector constructor, count, and signed 32-bit elements.
     *
     * @param value Elements to append.
     */
    void write_int32_vector(const std::vector<std::int32_t>& value);

    /**
     * @brief Returns the serialized buffer accumulated so far.
     *
     * @return A read-only reference to the serialized bytes.
     */
    const std::vector<std::uint8_t>& data() const;

private:
    std::vector<std::uint8_t> data_;
};

/**
 * @brief Reads primitive values from a Telegram TL wire-format buffer.
 */
class TlReader {
public:
    /**
     * @brief Creates a reader positioned at the beginning of a buffer.
     *
     * @param data Serialized TL bytes to read.
     */
    explicit TlReader(const std::vector<std::uint8_t>& data);

    /**
     * @brief Reads a signed 32-bit little-endian integer.
     *
     * @return The next signed 32-bit integer.
     * @throws std::out_of_range if fewer than four bytes remain.
     */
    std::int32_t read_int32();

    /**
     * @brief Reads a signed 64-bit little-endian integer.
     *
     * @return The next signed 64-bit integer.
     * @throws std::out_of_range if fewer than eight bytes remain.
     */
    std::int64_t read_int64();

    /**
     * @brief Reads a TL-encoded string.
     *
     * @return The next byte string interpreted as a string.
     * @throws std::out_of_range if the encoded value or its padding is truncated.
     */
    std::string read_string();

    /**
     * @brief Reads a TL-encoded byte string.
     *
     * @return The next byte string without its length prefix or padding.
     * @throws std::out_of_range if the encoded value or its padding is truncated.
     */
    std::vector<std::uint8_t> read_bytes();

    /**
     * @brief Reads a TL vector of signed 32-bit integers.
     *
     * @return The decoded vector elements.
     * @throws std::invalid_argument if the vector constructor is invalid.
     * @throws std::out_of_range if the vector length or element data is invalid.
     */
    std::vector<std::int32_t> read_int32_vector();

    /**
     * @brief Tests whether all input bytes have been consumed.
     *
     * @return `true` when the reader is positioned at the end of the buffer.
     */
    bool empty() const;

private:
    std::vector<std::uint8_t> data_;
    std::size_t offset_;

    void require(std::size_t size) const;
    std::vector<std::uint8_t> read_tl_bytes();
};

}

#endif
