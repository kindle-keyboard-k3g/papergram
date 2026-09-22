# MTProto 2.0 Client & Cryptography Engine

The MTProto subsystem (`src/mtproto/`) implements the Telegram MTProto 2.0 Mobile Protocol specification with **zero external dependencies** (no OpenSSL, Libsodium, or TDLib). All cryptographic primitives, key exchanges, and binary serialization are built from first principles in standard C++17.

---

## 1. Cryptographic Primitives (`src/mtproto/crypto.h`)

### SHA-1 & SHA-256
Native implementations complying with FIPS 180-4:
- `void sha1(const uint8_t* data, size_t len, uint8_t digest[20])`
- `void sha256(const uint8_t* data, size_t len, uint8_t digest[32])`
- Process 512-bit message blocks with round constants and padding.

### AES-256-IGE (Infinite Garble Extension)
Telegram MTProto requires AES-256 in IGE mode:
- `void aes256_ige_encrypt(const uint8_t* in, uint8_t* out, size_t len, const uint8_t* key, const uint8_t* iv)`
- `void aes256_ige_decrypt(const uint8_t* in, uint8_t* out, size_t len, const uint8_t* key, const uint8_t* iv)`
- Implemented with an optimized internal Rijndael S-box substitution and state permutation engine.

### BigInteger & Modular Exponentiation
Lightweight multi-precision arithmetic library capable of handling 2048-bit numbers:
- Implements `BigInteger` representation as an array of 32-bit or 64-bit limbs.
- `BigInteger mod_exp(const BigInteger& base, const BigInteger& exp, const BigInteger& mod)`: Montgomery multiplication or binary exponentiation for Diffie-Hellman key exchange.

---

## 2. Authentication Handshake (`src/mtproto/handshake.h`)

MTProto 2.0 client authorization performs a multi-step Diffie-Hellman handshake with Telegram's authentication servers:

```
Client                                                  Telegram Server
  |                                                           |
  | -------- 1. req_pq_multi(nonce) -----------------------> |
  |                                                           |
  | <------- 2. resPQ(nonce, server_nonce, pq, fingerprints)- |
  |                                                           |
  | [Pollard's Rho PQ Factorization: p, q = factor(pq)]       |
  | [Generate new_nonce, compute DH inner data]               |
  | [Encrypt with Server RSA Public Key]                      |
  |                                                           |
  | -------- 3. req_DH_params(p, q, encrypted_data) --------> |
  |                                                           |
  | <------- 4. server_DH_params_ok(g, dh_prime, g_a) ------- |
  |                                                           |
  | [Generate random client private b; compute g_b = g^b mod p]|
  | [Compute shared auth_key = (g_a)^b mod dh_prime]          |
  |                                                           |
  | -------- 5. set_client_DH_params(encrypted_client_data) -> |
  |                                                           |
  | <------- 6. dh_gen_ok(nonce, server_nonce, new_nonce_hash)-|
```

### Pollard's Rho Factorization
- Factorizes Telegram's 64-bit semi-prime `pq` into primes `p` and `q` (`p < q`).
- Optimized cycle-finding algorithm running in <50ms on the Kindle's 532 MHz ARM processor.

### Auth Key Derivation
- Generates 2048-bit `auth_key`.
- Derives `auth_key_id` as the lower 64 bits of `SHA1(auth_key)`.

---

## 3. TL Codec (Type Language) (`src/mtproto/tl_codec.h`)

Implements binary serialization and deserialization of Telegram Type Language schemas:
- **Scalar Primitives**:
  - `writeInt32(int32_t val)` / `readInt32()`
  - `writeInt64(int64_t val)` / `readInt64()`
  - `writeDouble(double val)` / `readDouble()`
- **TL Strings & Byte Arrays**:
  - Encodes lengths: 1 byte for length < 254; 4 bytes (`0xFE` prefix) for length >= 254.
  - Pads to 4-byte boundaries according to the TL specification.
- **Vectors & Constructors**:
  - Serializes vector magic `0x1cb5c415` followed by element count and serialized elements.

---

## 4. Session Storage (`src/mtproto/session_storage.h`)

Persists authentication keys and session parameters to the Kindle internal storage partition (`/mnt/us/telegram/session.dat`):
- Stores:
  - `auth_key` (256 bytes)
  - `auth_key_id` (8 bytes)
  - `server_salt` (8 bytes)
  - `session_id` (8 bytes)
  - `seq_no` (4 bytes)
- Provides atomic file writes (writing to temporary `.tmp` file and renaming) to prevent corruption during unexpected power loss.

---

## 5. High-Level Telegram Client (`src/mtproto/telegram_client.h`)

Facade coordinating MTProto RPC transactions:
- `bool sendCode(const PhoneNumber& phone, std::string& phoneCodeHash)`: Requests SMS/app authentication code.
- `bool signIn(const PhoneNumber& phone, const std::string& phoneCodeHash, const AuthCode& code)`: Completes login.
- `bool checkPassword(const std::string& password)`: Authenticates 2FA password via SRP (Secure Remote Password).
- `bool getDialogs(int offset, int limit, ChatList& outDialogs)`: Fetches user's chat list.
- `bool getHistory(const ChatId& chat, int offset, int limit, MessageHistory& outHistory)`: Fetches message history.
- `bool sendMessage(const ChatId& chat, const MessageText& text)`: Sends outgoing text message.
