#include "handshake.h"

#include "tl_codec.h"

#include <stdexcept>

namespace mtproto {
namespace {

void require_nonce(const std::vector<std::uint8_t>& nonce) {
    if (nonce.size() != 16U) {
        throw std::invalid_argument("MTProto nonce must contain 16 bytes");
    }
}

std::uint64_t greatest_common_divisor(std::uint64_t left, std::uint64_t right) {
    while (right != 0U) {
        const std::uint64_t remainder = left % right;
        left = right;
        right = remainder;
    }
    return left;
}

std::uint64_t add_mod(std::uint64_t left, std::uint64_t right, std::uint64_t modulus) {
    return left >= modulus - right ? left - (modulus - right) : left + right;
}

std::uint64_t multiply_mod(std::uint64_t left, std::uint64_t right, std::uint64_t modulus) {
    std::uint64_t result = 0U;
    left %= modulus;
    while (right != 0U) {
        if ((right & 1U) != 0U) {
            result = add_mod(result, left, modulus);
        }
        left = add_mod(left, left, modulus);
        right >>= 1U;
    }
    return result;
}

std::uint64_t power_mod(std::uint64_t base, std::uint64_t exponent, std::uint64_t modulus) {
    std::uint64_t result = 1U % modulus;
    base %= modulus;
    while (exponent != 0U) {
        if ((exponent & 1U) != 0U) {
            result = multiply_mod(result, base, modulus);
        }
        base = multiply_mod(base, base, modulus);
        exponent >>= 1U;
    }
    return result;
}

bool is_prime(std::uint64_t value) {
    if (value < 2U) {
        return false;
    }
    for (const std::uint64_t small : {2U, 3U, 5U, 7U, 11U, 13U, 17U, 19U, 23U, 29U, 31U, 37U}) {
        if (value == small) {
            return true;
        }
        if ((value % small) == 0U) {
            return false;
        }
    }
    std::uint64_t odd = value - 1U;
    unsigned powers = 0U;
    while ((odd & 1U) == 0U) {
        odd >>= 1U;
        ++powers;
    }
    for (const std::uint64_t base : {2U, 325U, 9375U, 28178U, 450775U, 9780504U, 1795265022U}) {
        if ((base % value) == 0U) {
            continue;
        }
        std::uint64_t witness = power_mod(base, odd, value);
        if (witness == 1U || witness == value - 1U) {
            continue;
        }
        unsigned power = 1U;
        while (power < powers && witness != value - 1U) {
            witness = multiply_mod(witness, witness, value);
            ++power;
        }
        if (witness != value - 1U) {
            return false;
        }
    }
    return true;
}

std::uint64_t pollard_rho(std::uint64_t value) {
    if ((value % 2U) == 0U) {
        return 2U;
    }
    for (std::uint64_t constant = 1U; constant < value; ++constant) {
        std::uint64_t slow = 2U;
        std::uint64_t fast = 2U;
        std::uint64_t divisor = 1U;
        while (divisor == 1U) {
            slow = add_mod(multiply_mod(slow, slow, value), constant, value);
            fast = add_mod(multiply_mod(fast, fast, value), constant, value);
            fast = add_mod(multiply_mod(fast, fast, value), constant, value);
            const std::uint64_t distance = slow > fast ? slow - fast : fast - slow;
            divisor = greatest_common_divisor(distance, value);
        }
        if (divisor != value) {
            return divisor;
        }
    }
    throw std::invalid_argument("unable to factor pq");
}

void append_little_endian(std::vector<std::uint8_t>& output, std::uint64_t value,
                          unsigned width) {
    for (unsigned index = 0U; index < width; ++index) {
        output.push_back(static_cast<std::uint8_t>(value >> (index * 8U)));
    }
}

void append_int256(std::vector<std::uint8_t>& output, std::uint64_t value) {
    append_little_endian(output, value, 32U);
}

}

PqFactors factorize_pq(std::uint64_t pq) {
    if (pq < 4U || is_prime(pq)) {
        throw std::invalid_argument("pq must have two factors");
    }
    const std::uint64_t factor = pollard_rho(pq);
    const std::uint64_t other = pq / factor;
    return factor < other ? PqFactors{factor, other} : PqFactors{other, factor};
}

BigInteger derive_shared_key(const BigInteger& peer_public,
                             const BigInteger& private_exponent,
                             const BigInteger& dh_prime) {
    return modular_exponentiation(peer_public, private_exponent, dh_prime);
}

MtprotoHandshake::MtprotoHandshake(const BigInteger& dh_prime, const BigInteger& generator)
    : dh_prime_(dh_prime), generator_(generator) {}

BigInteger MtprotoHandshake::client_public(const BigInteger& private_exponent) const {
    return modular_exponentiation(generator_, private_exponent, dh_prime_);
}

BigInteger MtprotoHandshake::auth_key(const BigInteger& server_public,
                                       const BigInteger& client_private) const {
    return derive_shared_key(server_public, client_private, dh_prime_);
}

BigInteger MtprotoHandshake::generate_auth_key(const BigInteger& server_public,
                                                const BigInteger& client_private) const {
    return auth_key(server_public, client_private);
}

std::vector<std::uint8_t> MtprotoHandshake::req_pq_multi(const std::vector<std::uint8_t>& nonce) const {
    TlWriter writer;
    writer.write_int32(static_cast<std::int32_t>(0xbe7e8ef1U));
    require_nonce(nonce);
    const std::vector<std::uint8_t>& output = writer.data();
    std::vector<std::uint8_t> result(output.begin(), output.end());
    result.insert(result.end(), nonce.begin(), nonce.end());
    return result;
}

std::vector<std::uint8_t> MtprotoHandshake::req_dh_params(
    const std::vector<std::uint8_t>& nonce,
    const std::vector<std::uint8_t>& server_nonce,
    const PqFactors& factors,
    const std::vector<std::uint8_t>& encrypted_data) const {
    require_nonce(nonce);
    require_nonce(server_nonce);
    TlWriter writer;
    writer.write_int32(static_cast<std::int32_t>(0xd712e4beU));
    std::vector<std::uint8_t> result(writer.data());
    result.insert(result.end(), nonce.begin(), nonce.end());
    result.insert(result.end(), server_nonce.begin(), server_nonce.end());
    append_int256(result, factors.first);
    append_int256(result, factors.second);
    append_little_endian(result, 0U, 8U);
    TlWriter payload;
    payload.write_bytes(encrypted_data);
    result.insert(result.end(), payload.data().begin(), payload.data().end());
    return result;
}

std::vector<std::uint8_t> MtprotoHandshake::set_client_dh_params(
    const std::vector<std::uint8_t>& nonce,
    const std::vector<std::uint8_t>& server_nonce,
    const std::vector<std::uint8_t>& encrypted_data) const {
    require_nonce(nonce);
    require_nonce(server_nonce);
    TlWriter writer;
    writer.write_int32(static_cast<std::int32_t>(0xf5045f1fU));
    std::vector<std::uint8_t> result(writer.data());
    result.insert(result.end(), nonce.begin(), nonce.end());
    result.insert(result.end(), server_nonce.begin(), server_nonce.end());
    TlWriter payload;
    payload.write_bytes(encrypted_data);
    result.insert(result.end(), payload.data().begin(), payload.data().end());
    return result;
}

}
