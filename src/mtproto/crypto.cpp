#include "crypto.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <stdexcept>

namespace mtproto {
namespace {

std::uint32_t rotate_left(std::uint32_t value, unsigned count) {
    return (value << count) | (value >> (32U - count));
}

std::uint32_t rotate_right(std::uint32_t value, unsigned count) {
    return (value >> count) | (value << (32U - count));
}

std::uint32_t read_big_endian(const std::uint8_t* value) {
    return (static_cast<std::uint32_t>(value[0]) << 24U) |
           (static_cast<std::uint32_t>(value[1]) << 16U) |
           (static_cast<std::uint32_t>(value[2]) << 8U) |
           static_cast<std::uint32_t>(value[3]);
}

void write_big_endian(std::uint8_t* output, std::uint32_t value) {
    output[0] = static_cast<std::uint8_t>(value >> 24U);
    output[1] = static_cast<std::uint8_t>(value >> 16U);
    output[2] = static_cast<std::uint8_t>(value >> 8U);
    output[3] = static_cast<std::uint8_t>(value);
}

void write_big_endian_64(std::uint8_t* output, std::uint64_t value) {
    for (unsigned index = 0; index < 8U; ++index) {
        output[7U - index] = static_cast<std::uint8_t>(value >> (index * 8U));
    }
}

const std::array<std::uint32_t, 64> sha256_round_constants = {
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
    0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
    0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
    0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
    0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
    0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
    0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
    0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
    0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
    0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
    0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
    0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
    0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
    0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
};

std::uint32_t sha256_constant(unsigned index) {
    return sha256_round_constants[index];
}

const std::array<std::uint8_t, 256> aes_sbox = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,0xb7,
    0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,0x04,
    0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,0x09,
    0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

std::uint8_t inverse_sbox(std::uint8_t value) {
    for (std::size_t index = 0; index < aes_sbox.size(); ++index) {
        if (aes_sbox[index] == value) {
            return static_cast<std::uint8_t>(index);
        }
    }
    return 0;
}

std::uint8_t multiply_byte(std::uint8_t left, std::uint8_t right) {
    std::uint8_t result = 0;
    for (unsigned index = 0; index < 8U; ++index) {
        if ((right & 1U) != 0U) {
            result ^= left;
        }
        const bool high = (left & 0x80U) != 0U;
        left = static_cast<std::uint8_t>(left << 1U);
        if (high) {
            left ^= 0x1bU;
        }
        right = static_cast<std::uint8_t>(right >> 1U);
    }
    return result;
}

using AesBlock = std::array<std::uint8_t, 16>;
using AesKey = std::array<std::uint32_t, 60>;

std::uint32_t sub_word(std::uint32_t value) {
    return (static_cast<std::uint32_t>(aes_sbox[(value >> 24U) & 0xffU]) << 24U) |
           (static_cast<std::uint32_t>(aes_sbox[(value >> 16U) & 0xffU]) << 16U) |
           (static_cast<std::uint32_t>(aes_sbox[(value >> 8U) & 0xffU]) << 8U) |
           aes_sbox[value & 0xffU];
}

std::uint32_t rotate_word(std::uint32_t value) {
    return (value << 8U) | (value >> 24U);
}

AesKey expand_key(const std::vector<std::uint8_t>& key) {
    if (key.size() != 32U) {
        throw std::invalid_argument("AES-256 requires a 32-byte key");
    }
    AesKey expanded{};
    for (unsigned index = 0; index < 8U; ++index) {
        expanded[index] = read_big_endian(key.data() + index * 4U);
    }
    std::uint32_t round_constant = 1U;
    for (unsigned index = 8U; index < 60U; ++index) {
        std::uint32_t word = expanded[index - 1U];
        if ((index % 8U) == 0U) {
            word = sub_word(rotate_word(word)) ^ (round_constant << 24U);
            round_constant = static_cast<std::uint32_t>(multiply_byte(static_cast<std::uint8_t>(round_constant), 2U));
        }
        if ((index % 8U) == 4U) {
            word = sub_word(word);
        }
        expanded[index] = expanded[index - 8U] ^ word;
    }
    return expanded;
}

void add_round_key(AesBlock& block, const AesKey& key, unsigned round) {
    for (unsigned column = 0; column < 4U; ++column) {
        const std::uint32_t word = key[round * 4U + column];
        block[column * 4U] ^= static_cast<std::uint8_t>(word >> 24U);
        block[column * 4U + 1U] ^= static_cast<std::uint8_t>(word >> 16U);
        block[column * 4U + 2U] ^= static_cast<std::uint8_t>(word >> 8U);
        block[column * 4U + 3U] ^= static_cast<std::uint8_t>(word);
    }
}

void substitute(AesBlock& block) {
    for (std::uint8_t& byte : block) {
        byte = aes_sbox[byte];
    }
}

void inverse_substitute(AesBlock& block) {
    for (std::uint8_t& byte : block) {
        byte = inverse_sbox(byte);
    }
}

void shift_rows(AesBlock& block) {
    AesBlock copy = block;
    for (unsigned row = 1U; row < 4U; ++row) {
        for (unsigned column = 0; column < 4U; ++column) {
            block[column * 4U + row] = copy[((column + row) % 4U) * 4U + row];
        }
    }
}

void inverse_shift_rows(AesBlock& block) {
    AesBlock copy = block;
    for (unsigned row = 1U; row < 4U; ++row) {
        for (unsigned column = 0; column < 4U; ++column) {
            block[column * 4U + row] = copy[((column + 4U - row) % 4U) * 4U + row];
        }
    }
}

void mix_columns(AesBlock& block) {
    for (unsigned column = 0; column < 4U; ++column) {
        const unsigned offset = column * 4U;
        const std::uint8_t a = block[offset];
        const std::uint8_t b = block[offset + 1U];
        const std::uint8_t c = block[offset + 2U];
        const std::uint8_t d = block[offset + 3U];
        block[offset] = multiply_byte(a, 2U) ^ multiply_byte(b, 3U) ^ c ^ d;
        block[offset + 1U] = a ^ multiply_byte(b, 2U) ^ multiply_byte(c, 3U) ^ d;
        block[offset + 2U] = a ^ b ^ multiply_byte(c, 2U) ^ multiply_byte(d, 3U);
        block[offset + 3U] = multiply_byte(a, 3U) ^ b ^ c ^ multiply_byte(d, 2U);
    }
}

void inverse_mix_columns(AesBlock& block) {
    for (unsigned column = 0; column < 4U; ++column) {
        const unsigned offset = column * 4U;
        const std::uint8_t a = block[offset];
        const std::uint8_t b = block[offset + 1U];
        const std::uint8_t c = block[offset + 2U];
        const std::uint8_t d = block[offset + 3U];
        block[offset] = multiply_byte(a, 14U) ^ multiply_byte(b, 11U) ^ multiply_byte(c, 13U) ^ multiply_byte(d, 9U);
        block[offset + 1U] = multiply_byte(a, 9U) ^ multiply_byte(b, 14U) ^ multiply_byte(c, 11U) ^ multiply_byte(d, 13U);
        block[offset + 2U] = multiply_byte(a, 13U) ^ multiply_byte(b, 9U) ^ multiply_byte(c, 14U) ^ multiply_byte(d, 11U);
        block[offset + 3U] = multiply_byte(a, 11U) ^ multiply_byte(b, 13U) ^ multiply_byte(c, 9U) ^ multiply_byte(d, 14U);
    }
}

AesBlock encrypt_block(const AesBlock& input, const AesKey& key) {
    AesBlock block = input;
    add_round_key(block, key, 0U);
    for (unsigned round = 1U; round < 14U; ++round) {
        substitute(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, key, round);
    }
    substitute(block);
    shift_rows(block);
    add_round_key(block, key, 14U);
    return block;
}

AesBlock decrypt_block(const AesBlock& input, const AesKey& key) {
    AesBlock block = input;
    add_round_key(block, key, 14U);
    for (unsigned round = 13U; round > 0U; --round) {
        inverse_shift_rows(block);
        inverse_substitute(block);
        add_round_key(block, key, round);
        inverse_mix_columns(block);
    }
    inverse_shift_rows(block);
    inverse_substitute(block);
    add_round_key(block, key, 0U);
    return block;
}

std::vector<std::uint8_t> ige_transform(const std::vector<std::uint8_t>& input,
                                        const std::vector<std::uint8_t>& key,
                                        const std::vector<std::uint8_t>& iv,
                                        bool decrypt) {
    if ((input.size() % 16U) != 0U) {
        throw std::invalid_argument("AES-IGE input must be a multiple of 16 bytes");
    }
    if (iv.size() != 32U) {
        throw std::invalid_argument("AES-IGE requires a 32-byte IV");
    }
    const AesKey expanded = expand_key(key);
    AesBlock previous_cipher{};
    AesBlock previous_plain{};
    std::copy(iv.begin(), iv.begin() + 16, previous_cipher.begin());
    std::copy(iv.begin() + 16, iv.end(), previous_plain.begin());
    std::vector<std::uint8_t> output(input.size());
    for (std::size_t offset = 0; offset < input.size(); offset += 16U) {
        AesBlock block{};
        std::copy(input.begin() + offset, input.begin() + offset + 16U, block.begin());
        AesBlock transformed = block;
        for (unsigned index = 0; index < 16U; ++index) {
            transformed[index] ^= decrypt ? previous_plain[index] : previous_cipher[index];
        }
        transformed = decrypt ? decrypt_block(transformed, expanded) : encrypt_block(transformed, expanded);
        for (unsigned index = 0; index < 16U; ++index) {
            transformed[index] ^= decrypt ? previous_cipher[index] : previous_plain[index];
        }
        std::copy(transformed.begin(), transformed.end(), output.begin() + offset);
        if (decrypt) {
            previous_cipher = block;
            previous_plain = transformed;
        }
        if (!decrypt) {
            previous_cipher = transformed;
            previous_plain = block;
        }
    }
    return output;
}

std::vector<std::uint8_t> trim_integer(std::vector<std::uint8_t> value) {
    while (value.size() > 1U && value.back() == 0U) {
        value.pop_back();
    }
    if (value.empty()) {
        value.push_back(0U);
    }
    return value;
}

int compare_integer(const std::vector<std::uint8_t>& left, const std::vector<std::uint8_t>& right) {
    if (left.size() != right.size()) {
        return left.size() < right.size() ? -1 : 1;
    }
    for (std::size_t index = left.size(); index > 0U; --index) {
        if (left[index - 1U] != right[index - 1U]) {
            return left[index - 1U] < right[index - 1U] ? -1 : 1;
        }
    }
    return 0;
}

std::vector<std::uint8_t> subtract_integer(std::vector<std::uint8_t> left,
                                           const std::vector<std::uint8_t>& right) {
    int borrow = 0;
    for (std::size_t index = 0; index < left.size(); ++index) {
        const int result = static_cast<int>(left[index]) - (index < right.size() ? right[index] : 0) - borrow;
        left[index] = static_cast<std::uint8_t>(result < 0 ? result + 256 : result);
        borrow = result < 0 ? 1 : 0;
    }
    return trim_integer(left);
}

std::vector<std::uint8_t> multiply_integer(const std::vector<std::uint8_t>& left,
                                           const std::vector<std::uint8_t>& right) {
    std::vector<std::uint8_t> result(left.size() + right.size(), 0U);
    for (std::size_t left_index = 0; left_index < left.size(); ++left_index) {
        unsigned carry = 0U;
        for (std::size_t right_index = 0; right_index < right.size(); ++right_index) {
            const unsigned value = result[left_index + right_index] +
                                   static_cast<unsigned>(left[left_index]) * right[right_index] + carry;
            result[left_index + right_index] = static_cast<std::uint8_t>(value);
            carry = value >> 8U;
        }
        std::size_t index = left_index + right.size();
        while (carry != 0U) {
            const unsigned value = result[index] + carry;
            result[index] = static_cast<std::uint8_t>(value);
            carry = value >> 8U;
            ++index;
            if (index == result.size() && carry != 0U) {
                result.push_back(0U);
            }
        }
    }
    return trim_integer(result);
}

std::vector<std::uint8_t> shift_left_one(const std::vector<std::uint8_t>& value) {
    std::vector<std::uint8_t> result(value.size(), 0U);
    unsigned carry = 0U;
    for (std::size_t index = 0; index < value.size(); ++index) {
        const unsigned shifted = (static_cast<unsigned>(value[index]) << 1U) | carry;
        result[index] = static_cast<std::uint8_t>(shifted);
        carry = shifted >> 8U;
    }
    if (carry != 0U) {
        result.push_back(static_cast<std::uint8_t>(carry));
    }
    return trim_integer(result);
}

std::vector<std::uint8_t> add_small(std::vector<std::uint8_t> value, unsigned amount) {
    std::size_t index = 0U;
    while (amount != 0U) {
        if (index == value.size()) {
            value.push_back(0U);
        }
        const unsigned sum = value[index] + (amount & 0xffU);
        value[index] = static_cast<std::uint8_t>(sum);
        amount = (amount >> 8U) + (sum >> 8U);
        ++index;
    }
    return trim_integer(value);
}

std::vector<std::uint8_t> modulo_integer(const std::vector<std::uint8_t>& value,
                                         const std::vector<std::uint8_t>& modulus) {
    if (modulus.size() == 1U && modulus[0] == 0U) {
        throw std::invalid_argument("modulus must not be zero");
    }
    std::vector<std::uint8_t> remainder(1U, 0U);
    for (std::size_t index = value.size(); index > 0U; --index) {
        for (int bit = 7; bit >= 0; --bit) {
            remainder = shift_left_one(remainder);
            if ((value[index - 1U] & (1U << bit)) != 0U) {
                remainder = add_small(remainder, 1U);
            }
            if (compare_integer(remainder, modulus) >= 0) {
                remainder = subtract_integer(remainder, modulus);
            }
        }
    }
    return remainder;
}

bool is_zero(const std::vector<std::uint8_t>& value) {
    return value.size() == 1U && value[0] == 0U;
}

bool is_odd(const std::vector<std::uint8_t>& value) {
    return (value[0] & 1U) != 0U;
}

std::vector<std::uint8_t> divide_by_two(std::vector<std::uint8_t> value) {
    unsigned carry = 0U;
    for (std::size_t index = value.size(); index > 0U; --index) {
        const unsigned current = (carry << 8U) | value[index - 1U];
        value[index - 1U] = static_cast<std::uint8_t>(current / 2U);
        carry = current & 1U;
    }
    return trim_integer(value);
}

struct Sha1RoundParams {
    std::uint32_t function;
    std::uint32_t constant;
};

Sha1RoundParams sha1_round_params(unsigned index, std::uint32_t b, std::uint32_t c, std::uint32_t d) {
    if (index < 20U) {
        return {(b & c) | ((~b) & d), 0x5a827999U};
    }
    if (index < 40U) {
        return {b ^ c ^ d, 0x6ed9eba1U};
    }
    if (index < 60U) {
        return {(b & c) | (b & d) | (c & d), 0x8f1bbcdcU};
    }
    return {b ^ c ^ d, 0xca62c1d6U};
}

}

std::vector<std::uint8_t> sha1(const std::vector<std::uint8_t>& message) {
    std::vector<std::uint8_t> padded = message;
    padded.push_back(0x80U);
    while ((padded.size() % 64U) != 56U) {
        padded.push_back(0U);
    }
    const std::uint64_t bit_length = static_cast<std::uint64_t>(message.size()) * 8U;
    const std::size_t old_size = padded.size();
    padded.resize(old_size + 8U);
    write_big_endian_64(padded.data() + old_size, bit_length);
    std::uint32_t h0 = 0x67452301U;
    std::uint32_t h1 = 0xefcdab89U;
    std::uint32_t h2 = 0x98badcfeU;
    std::uint32_t h3 = 0x10325476U;
    std::uint32_t h4 = 0xc3d2e1f0U;
    for (std::size_t offset = 0; offset < padded.size(); offset += 64U) {
        std::array<std::uint32_t, 80> words{};
        for (unsigned index = 0; index < 16U; ++index) {
            words[index] = read_big_endian(padded.data() + offset + index * 4U);
        }
        for (unsigned index = 16U; index < 80U; ++index) {
            words[index] = rotate_left(words[index - 3U] ^ words[index - 8U] ^ words[index - 14U] ^ words[index - 16U], 1U);
        }
        std::uint32_t a = h0;
        std::uint32_t b = h1;
        std::uint32_t c = h2;
        std::uint32_t d = h3;
        std::uint32_t e = h4;
        for (unsigned index = 0; index < 80U; ++index) {
            const Sha1RoundParams params = sha1_round_params(index, b, c, d);
            const std::uint32_t temporary = rotate_left(a, 5U) + params.function + e + params.constant + words[index];
            e = d;
            d = c;
            c = rotate_left(b, 30U);
            b = a;
            a = temporary;
        }
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }
    std::vector<std::uint8_t> digest(20U);
    write_big_endian(digest.data(), h0);
    write_big_endian(digest.data() + 4U, h1);
    write_big_endian(digest.data() + 8U, h2);
    write_big_endian(digest.data() + 12U, h3);
    write_big_endian(digest.data() + 16U, h4);
    return digest;
}

std::vector<std::uint8_t> sha256(const std::vector<std::uint8_t>& message) {
    std::vector<std::uint8_t> padded = message;
    padded.push_back(0x80U);
    while ((padded.size() % 64U) != 56U) {
        padded.push_back(0U);
    }
    const std::uint64_t bit_length = static_cast<std::uint64_t>(message.size()) * 8U;
    const std::size_t old_size = padded.size();
    padded.resize(old_size + 8U);
    write_big_endian_64(padded.data() + old_size, bit_length);
    std::array<std::uint32_t, 8> state = {0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
                                          0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U};
    for (std::size_t offset = 0; offset < padded.size(); offset += 64U) {
        std::array<std::uint32_t, 64> words{};
        for (unsigned index = 0; index < 16U; ++index) {
            words[index] = read_big_endian(padded.data() + offset + index * 4U);
        }
        for (unsigned index = 16U; index < 64U; ++index) {
            const std::uint32_t first = rotate_right(words[index - 15U], 7U) ^ rotate_right(words[index - 15U], 18U) ^ (words[index - 15U] >> 3U);
            const std::uint32_t second = rotate_right(words[index - 2U], 17U) ^ rotate_right(words[index - 2U], 19U) ^ (words[index - 2U] >> 10U);
            words[index] = words[index - 16U] + first + words[index - 7U] + second;
        }
        std::uint32_t a = state[0];
        std::uint32_t b = state[1];
        std::uint32_t c = state[2];
        std::uint32_t d = state[3];
        std::uint32_t e = state[4];
        std::uint32_t f = state[5];
        std::uint32_t g = state[6];
        std::uint32_t h = state[7];
        for (unsigned index = 0; index < 64U; ++index) {
            const std::uint32_t upper = rotate_right(e, 6U) ^ rotate_right(e, 11U) ^ rotate_right(e, 25U);
            const std::uint32_t choose = (e & f) ^ ((~e) & g);
            const std::uint32_t temporary_one = h + upper + choose + sha256_constant(index) + words[index];
            const std::uint32_t lower = rotate_right(a, 2U) ^ rotate_right(a, 13U) ^ rotate_right(a, 22U);
            const std::uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
            const std::uint32_t temporary_two = lower + majority;
            h = g;
            g = f;
            f = e;
            e = d + temporary_one;
            d = c;
            c = b;
            b = a;
            a = temporary_one + temporary_two;
        }
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }
    std::vector<std::uint8_t> digest(32U);
    for (unsigned index = 0; index < 8U; ++index) {
        write_big_endian(digest.data() + index * 4U, state[index]);
    }
    return digest;
}

std::vector<std::uint8_t> aes256_ige_encrypt(const std::vector<std::uint8_t>& plain,
                                               const std::vector<std::uint8_t>& key,
                                               const std::vector<std::uint8_t>& iv) {
    return ige_transform(plain, key, iv, false);
}

std::vector<std::uint8_t> aes256_ige_decrypt(const std::vector<std::uint8_t>& cipher,
                                               const std::vector<std::uint8_t>& key,
                                               const std::vector<std::uint8_t>& iv) {
    return ige_transform(cipher, key, iv, true);
}

BigInteger::BigInteger(const std::vector<std::uint8_t>& value) : value_(trim_integer(value)) {}

BigInteger BigInteger::from_uint64(std::uint64_t value) {
    std::vector<std::uint8_t> bytes;
    do {
        bytes.push_back(static_cast<std::uint8_t>(value));
        value >>= 8U;
    } while (value != 0U);
    return BigInteger(bytes);
}

BigInteger BigInteger::from_bytes(const std::vector<std::uint8_t>& value) {
    std::vector<std::uint8_t> little(value.rbegin(), value.rend());
    return BigInteger(little);
}

std::vector<std::uint8_t> BigInteger::to_bytes() const {
    return std::vector<std::uint8_t>(value_.rbegin(), value_.rend());
}

std::uint64_t BigInteger::to_uint64() const {
    std::uint64_t result = 0U;
    const std::size_t count = std::min<std::size_t>(value_.size(), 8U);
    for (std::size_t index = count; index > 0U; --index) {
        result = (result << 8U) | value_[index - 1U];
    }
    return result;
}

bool BigInteger::operator==(const BigInteger& other) const {
    return value_ == other.value_;
}

BigInteger modular_exponentiation(const BigInteger& base,
                                  const BigInteger& exponent,
                                  const BigInteger& modulus) {
    if (is_zero(modulus.value_)) {
        throw std::invalid_argument("modulus must not be zero");
    }
    std::vector<std::uint8_t> result = modulo_integer({1U}, modulus.value_);
    std::vector<std::uint8_t> factor = modulo_integer(base.value_, modulus.value_);
    std::vector<std::uint8_t> remaining = exponent.value_;
    while (!is_zero(remaining)) {
        if (is_odd(remaining)) {
            result = modulo_integer(multiply_integer(result, factor), modulus.value_);
        }
        factor = modulo_integer(multiply_integer(factor, factor), modulus.value_);
        remaining = divide_by_two(remaining);
    }
    return BigInteger(result);
}

}
