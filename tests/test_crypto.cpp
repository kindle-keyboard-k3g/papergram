#include "test_framework.h"

#include "../src/mtproto/crypto.h"

#include <cstdint>
#include <string>
#include <vector>

namespace {

std::vector<std::uint8_t> bytes(const std::string& text) {
    return std::vector<std::uint8_t>(text.begin(), text.end());
}

std::string hex(const std::vector<std::uint8_t>& value) {
    static const char digits[] = "0123456789abcdef";
    std::string result;
    for (std::uint8_t byte : value) {
        result.push_back(digits[byte >> 4U]);
        result.push_back(digits[byte & 0x0fU]);
    }
    return result;
}

}

TEST(sha1_matches_empty_message_vector) {
    const std::vector<std::uint8_t> digest = mtproto::sha1(bytes(""));
    ASSERT_STR_EQ("da39a3ee5e6b4b0d3255bfef95601890afd80709", hex(digest));
}

TEST(sha256_matches_abc_vector) {
    const std::vector<std::uint8_t> digest = mtproto::sha256(bytes("abc"));
    ASSERT_STR_EQ("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", hex(digest));
}

TEST(aes256_ige_matches_two_block_vector) {
    const std::vector<std::uint8_t> key = [] {
        std::vector<std::uint8_t> value(32U);
        for (std::size_t index = 0; index < value.size(); ++index) {
            value[index] = static_cast<std::uint8_t>(index);
        }
        return value;
    }();
    const std::vector<std::uint8_t> iv = key;
    const std::vector<std::uint8_t> plain = [] {
        std::vector<std::uint8_t> value(32U);
        for (std::size_t index = 0; index < value.size(); ++index) {
            value[index] = static_cast<std::uint8_t>(index + 32U);
        }
        return value;
    }();
    const std::vector<std::uint8_t> encrypted = mtproto::aes256_ige_encrypt(plain, key, iv);
    ASSERT_STR_EQ("62c64e3a554cecd5925aec67721af054ce199f74c3fb179bff617dd69420dc89", hex(encrypted));
    ASSERT_EQ(plain, mtproto::aes256_ige_decrypt(encrypted, key, iv));
}

TEST(big_integer_modular_exponentiation_matches_dh_example) {
    const mtproto::BigInteger base = mtproto::BigInteger::from_uint64(5U);
    const mtproto::BigInteger exponent = mtproto::BigInteger::from_uint64(117U);
    const mtproto::BigInteger modulus = mtproto::BigInteger::from_uint64(19U);
    const mtproto::BigInteger result = mtproto::modular_exponentiation(base, exponent, modulus);
    ASSERT_EQ(1U, result.to_uint64());
}
