#ifndef KINDLE_MTPROTO_CRYPTO_H
#define KINDLE_MTPROTO_CRYPTO_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mtproto {

class BigInteger {
public:
    static BigInteger from_uint64(std::uint64_t value);
    static BigInteger from_bytes(const std::vector<std::uint8_t>& value);

    std::vector<std::uint8_t> to_bytes() const;
    std::uint64_t to_uint64() const;
    bool operator==(const BigInteger& other) const;

private:
    explicit BigInteger(const std::vector<std::uint8_t>& value);
    std::vector<std::uint8_t> value_;

    friend BigInteger modular_exponentiation(const BigInteger&, const BigInteger&, const BigInteger&);
};

std::vector<std::uint8_t> sha1(const std::vector<std::uint8_t>& message);
std::vector<std::uint8_t> sha256(const std::vector<std::uint8_t>& message);
std::vector<std::uint8_t> aes256_ige_encrypt(const std::vector<std::uint8_t>& plain,
                                               const std::vector<std::uint8_t>& key,
                                               const std::vector<std::uint8_t>& iv);
std::vector<std::uint8_t> aes256_ige_decrypt(const std::vector<std::uint8_t>& cipher,
                                               const std::vector<std::uint8_t>& key,
                                               const std::vector<std::uint8_t>& iv);
BigInteger modular_exponentiation(const BigInteger& base,
                                  const BigInteger& exponent,
                                  const BigInteger& modulus);

}

#endif
