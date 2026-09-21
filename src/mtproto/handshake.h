#ifndef KINDLE_MTPROTO_HANDSHAKE_H
#define KINDLE_MTPROTO_HANDSHAKE_H

#include "crypto.h"

#include <cstdint>
#include <vector>

namespace mtproto {

struct PqFactors {
    std::uint64_t first;
    std::uint64_t second;
};

PqFactors factorize_pq(std::uint64_t pq);
BigInteger derive_shared_key(const BigInteger& peer_public,
                             const BigInteger& private_exponent,
                             const BigInteger& dh_prime);

class MtprotoHandshake {
public:
    MtprotoHandshake(const BigInteger& dh_prime, const BigInteger& generator);

    BigInteger client_public(const BigInteger& private_exponent) const;
    BigInteger auth_key(const BigInteger& server_public,
                        const BigInteger& client_private) const;
    BigInteger generate_auth_key(const BigInteger& server_public,
                                 const BigInteger& client_private) const;
    std::vector<std::uint8_t> req_pq_multi(const std::vector<std::uint8_t>& nonce) const;
    std::vector<std::uint8_t> req_dh_params(const std::vector<std::uint8_t>& nonce,
                                            const std::vector<std::uint8_t>& server_nonce,
                                            const PqFactors& factors,
                                            const std::vector<std::uint8_t>& encrypted_data) const;
    std::vector<std::uint8_t> set_client_dh_params(const std::vector<std::uint8_t>& nonce,
                                                   const std::vector<std::uint8_t>& server_nonce,
                                                   const std::vector<std::uint8_t>& encrypted_data) const;

private:
    BigInteger dh_prime_;
    BigInteger generator_;
};

}

#endif
