#ifndef KINDLE_MTPROTO_CRYPTO_H
#define KINDLE_MTPROTO_CRYPTO_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mtproto {

/**
 * @brief Represents a non-negative arbitrary-precision integer.
 *
 * Byte conversions use big-endian order, while arithmetic is performed on the
 * normalized internal representation.
 */
class BigInteger {
public:
    /**
     * @brief Creates an integer from an unsigned 64-bit value.
     *
     * @param value Value to represent.
     * @return The corresponding non-negative integer.
     */
    static BigInteger from_uint64(std::uint64_t value);

    /**
     * @brief Creates an integer from a big-endian byte sequence.
     *
     * @param value Big-endian bytes; an empty sequence represents zero.
     * @return The corresponding non-negative integer.
     */
    static BigInteger from_bytes(const std::vector<std::uint8_t>& value);

    /**
     * @brief Converts the integer to big-endian bytes.
     *
     * @return A normalized big-endian byte sequence.
     */
    std::vector<std::uint8_t> to_bytes() const;

    /**
     * @brief Converts the integer to an unsigned 64-bit value.
     *
     * Values wider than 64 bits are truncated to their least significant
     * 64 bits.
     *
     * @return The least significant 64 bits of the integer.
     */
    std::uint64_t to_uint64() const;

    /**
     * @brief Compares two integers for equality.
     *
     * @param other Integer to compare with this instance.
     * @return `true` when both integers have the same value.
     */
    bool operator==(const BigInteger& other) const;

private:
    explicit BigInteger(const std::vector<std::uint8_t>& value);
    std::vector<std::uint8_t> value_;

    friend BigInteger modular_exponentiation(const BigInteger&, const BigInteger&, const BigInteger&);
};

/**
 * @brief Computes the SHA-1 digest of a byte sequence.
 *
 * @param message Message to hash.
 * @return The 20-byte SHA-1 digest.
 */
std::vector<std::uint8_t> sha1(const std::vector<std::uint8_t>& message);

/**
 * @brief Computes the SHA-256 digest of a byte sequence.
 *
 * @param message Message to hash.
 * @return The 32-byte SHA-256 digest.
 */
std::vector<std::uint8_t> sha256(const std::vector<std::uint8_t>& message);

/**
 * @brief Encrypts data with AES-256 in IGE mode.
 *
 * @param plain Plaintext whose size is a multiple of 16 bytes.
 * @param key 32-byte AES-256 key.
 * @param iv 32-byte IGE initialization vector.
 * @return Ciphertext with the same size as `plain`.
 * @throws std::invalid_argument if the key, IV, or plaintext size is invalid.
 */
std::vector<std::uint8_t> aes256_ige_encrypt(const std::vector<std::uint8_t>& plain,
                                               const std::vector<std::uint8_t>& key,
                                               const std::vector<std::uint8_t>& iv);

/**
 * @brief Decrypts data with AES-256 in IGE mode.
 *
 * @param cipher Ciphertext whose size is a multiple of 16 bytes.
 * @param key 32-byte AES-256 key.
 * @param iv 32-byte IGE initialization vector.
 * @return Plaintext with the same size as `cipher`.
 * @throws std::invalid_argument if the key, IV, or ciphertext size is invalid.
 */
std::vector<std::uint8_t> aes256_ige_decrypt(const std::vector<std::uint8_t>& cipher,
                                               const std::vector<std::uint8_t>& key,
                                               const std::vector<std::uint8_t>& iv);

/**
 * @brief Computes modular exponentiation for arbitrary-precision integers.
 *
 * @param base Base value.
 * @param exponent Non-negative exponent.
 * @param modulus Non-zero modulus.
 * @return `(base ^ exponent) mod modulus`.
 * @throws std::invalid_argument if `modulus` is zero.
 */
BigInteger modular_exponentiation(const BigInteger& base,
                                  const BigInteger& exponent,
                                  const BigInteger& modulus);

}

#endif
