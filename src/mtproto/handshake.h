#ifndef KINDLE_MTPROTO_HANDSHAKE_H
#define KINDLE_MTPROTO_HANDSHAKE_H

#include "crypto.h"

#include <cstdint>
#include <vector>

namespace mtproto {

/**
 * @brief Holds the two factors of an MTProto PQ value.
 */
struct PqFactors {
    /** @brief The smaller factor of PQ. */
    std::uint64_t first;

    /** @brief The larger factor of PQ. */
    std::uint64_t second;
};

/**
 * @brief Factorizes a composite MTProto PQ value.
 *
 * @param pq Composite value containing two factors.
 * @return The factors ordered from smallest to largest.
 * @throws std::invalid_argument if `pq` is too small, prime, or cannot be factored.
 */
PqFactors factorize_pq(std::uint64_t pq);

/**
 * @brief Derives a Diffie-Hellman shared key from a peer public key.
 *
 * @param peer_public Peer public DH value.
 * @param private_exponent Local private DH exponent.
 * @param dh_prime DH prime modulus.
 * @return The shared key `(peer_public ^ private_exponent) mod dh_prime`.
 * @throws std::invalid_argument if `dh_prime` is zero.
 */
BigInteger derive_shared_key(const BigInteger& peer_public,
                             const BigInteger& private_exponent,
                             const BigInteger& dh_prime);

/**
 * @brief Builds MTProto handshake messages and derives DH keys.
 */
class MtprotoHandshake {
public:
    /**
     * @brief Creates a handshake helper for a DH group.
     *
     * @param dh_prime DH prime modulus.
     * @param generator DH generator.
     */
    MtprotoHandshake(const BigInteger& dh_prime, const BigInteger& generator);

    /**
     * @brief Computes the client's DH public value.
     *
     * @param private_exponent Client's private DH exponent.
     * @return The public value `(generator ^ private_exponent) mod dh_prime`.
     * @throws std::invalid_argument if the configured DH prime is zero.
     */
    BigInteger client_public(const BigInteger& private_exponent) const;

    /**
     * @brief Computes the shared authentication key from the server public value.
     *
     * @param server_public Server's public DH value.
     * @param client_private Client's private DH exponent.
     * @return The shared authentication key.
     * @throws std::invalid_argument if the configured DH prime is zero.
     */
    BigInteger auth_key(const BigInteger& server_public,
                        const BigInteger& client_private) const;

    /**
     * @brief Computes the shared authentication key.
     *
     * This method is the explicit key-generation form of auth_key().
     *
     * @param server_public Server's public DH value.
     * @param client_private Client's private DH exponent.
     * @return The shared authentication key.
     * @throws std::invalid_argument if the configured DH prime is zero.
     */
    BigInteger generate_auth_key(const BigInteger& server_public,
                                 const BigInteger& client_private) const;

    /**
     * @brief Encodes an MTProto req_pq_multi request.
     *
     * @param nonce 16-byte client nonce.
     * @return The serialized request payload.
     * @throws std::invalid_argument if `nonce` is not exactly 16 bytes.
     */
    std::vector<std::uint8_t> req_pq_multi(const std::vector<std::uint8_t>& nonce) const;

    /**
     * @brief Encodes an MTProto req_DH_params request.
     *
     * @param nonce 16-byte client nonce.
     * @param server_nonce 16-byte server nonce.
     * @param factors Factors of the server's PQ value.
     * @param encrypted_data Encrypted inner DH parameters.
     * @return The serialized request payload.
     * @throws std::invalid_argument if either nonce is not exactly 16 bytes.
     */
    std::vector<std::uint8_t> req_dh_params(const std::vector<std::uint8_t>& nonce,
                                            const std::vector<std::uint8_t>& server_nonce,
                                            const PqFactors& factors,
                                            const std::vector<std::uint8_t>& encrypted_data) const;

    /**
     * @brief Encodes an MTProto set_client_DH_params request.
     *
     * @param nonce 16-byte client nonce.
     * @param server_nonce 16-byte server nonce.
     * @param encrypted_data Encrypted client DH parameters.
     * @return The serialized request payload.
     * @throws std::invalid_argument if either nonce is not exactly 16 bytes.
     */
    std::vector<std::uint8_t> set_client_dh_params(const std::vector<std::uint8_t>& nonce,
                                                   const std::vector<std::uint8_t>& server_nonce,
                                                   const std::vector<std::uint8_t>& encrypted_data) const;

private:
    BigInteger dh_prime_;
    BigInteger generator_;
};

}

#endif
