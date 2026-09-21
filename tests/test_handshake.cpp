#include "test_framework.h"

#include "../src/mtproto/handshake.h"
#include "../src/mtproto/session_storage.h"
#include "../src/mtproto/tl_codec.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

TEST(tl_codec_round_trips_scalars_and_bytes) {
    mtproto::TlWriter writer;
    writer.write_int32(0x12345678);
    writer.write_int64(0x0102030405060708LL);
    writer.write_string("hello");
    writer.write_bytes(std::vector<std::uint8_t>{1U, 2U, 3U});
    writer.write_int32_vector(std::vector<std::int32_t>{4, 5, 6});

    mtproto::TlReader reader(writer.data());
    ASSERT_EQ(0x12345678, reader.read_int32());
    ASSERT_EQ(0x0102030405060708LL, reader.read_int64());
    ASSERT_STR_EQ("hello", reader.read_string());
    ASSERT_EQ(std::vector<std::uint8_t>({1U, 2U, 3U}), reader.read_bytes());
    ASSERT_EQ(std::vector<std::int32_t>({4, 5, 6}), reader.read_int32_vector());
    ASSERT_TRUE(reader.empty());
}

TEST(tl_codec_uses_short_string_encoding_for_small_values) {
    mtproto::TlWriter writer;
    writer.write_string("abc");
    ASSERT_EQ(4U, writer.data().size());
    ASSERT_EQ(static_cast<std::uint8_t>(3U), writer.data().at(0));
}

TEST(pq_factorization_returns_ordered_factors) {
    const mtproto::PqFactors factors = mtproto::factorize_pq(1009U * 1013U);
    ASSERT_EQ(1009U, factors.first);
    ASSERT_EQ(1013U, factors.second);
}

TEST(handshake_derives_shared_auth_key_from_dh_values) {
    const mtproto::BigInteger prime = mtproto::BigInteger::from_uint64(23U);
    const mtproto::BigInteger generator = mtproto::BigInteger::from_uint64(5U);
    const mtproto::BigInteger client_private = mtproto::BigInteger::from_uint64(6U);
    const mtproto::BigInteger server_private = mtproto::BigInteger::from_uint64(15U);
    const mtproto::BigInteger server_public = mtproto::modular_exponentiation(generator, server_private, prime);
    const mtproto::BigInteger client_public = mtproto::modular_exponentiation(generator, client_private, prime);
    const mtproto::BigInteger client_shared = mtproto::derive_shared_key(server_public, client_private, prime);
    const mtproto::BigInteger server_shared = mtproto::derive_shared_key(client_public, server_private, prime);
    ASSERT_EQ(client_shared.to_uint64(), server_shared.to_uint64());
    ASSERT_EQ(2U, client_shared.to_uint64());
}

TEST(session_storage_round_trips_authentication_state) {
    const std::string path = "/tmp/kindle-session-test/session.dat";
    mtproto::SessionStorage storage(path);
    storage.remove();
    const mtproto::SessionData expected{{1U, 2U, 3U}, 4U, 5U, 2};
    ASSERT_TRUE(storage.save(expected));
    mtproto::SessionData actual;
    ASSERT_TRUE(storage.load(actual));
    ASSERT_EQ(expected.auth_key, actual.auth_key);
    ASSERT_EQ(expected.server_salt, actual.server_salt);
    ASSERT_EQ(expected.session_id, actual.session_id);
    ASSERT_EQ(expected.dc_id, actual.dc_id);
    ASSERT_TRUE(storage.remove());
}
