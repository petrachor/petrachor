/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Common.h
 * @author Alex Leverington <nessence@gmail.com>
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Ethereum-specific data structures & algorithms.
 */

#pragma once

#include <mutex>
#include <libdevcore/Address.h>
#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/Exceptions.h>
#include <libdevcore/FixedHash.h>
#include <libdevcore/SHA3.h>
#include "BLS12_381.h"

namespace dev
{

static const u256 c_secp256k1n("115792089237316195423570985008687907852837564279074904382605163141518161494337");

class BLS {
public:
    typedef BLS12_381::Scalar Secret;
    typedef FixedHash<BLS12_381::Scalar::size> NonceScalar;
    typedef BLS12_381::G2 Public;
    typedef BLS12_381::G1 Signature;
    struct SignatureStruct : public Signature
    {
        SignatureStruct() {}
        SignatureStruct(Signature const& s, Public const& _publicKey) : Signature(s), publicKey(_publicKey) { }
        SignatureStruct(RLP const& bytes);
        bool isValid() const noexcept;
        bool isZero() const;
        bool lowS() const { return false; }

        void streamRLP(RLPStream& _s) const;
        Public publicKey;
    };
};

class ECDSA {
public:
    typedef SecureFixedHash<32> Secret;
    typedef h512 Public;
    typedef h520 Signature;

    struct SignatureStruct
    {
        SignatureStruct() = default;
        SignatureStruct(Signature const& _s) { *(h520*)this = _s; }
        SignatureStruct(h256 const& _r, h256 const& _s, byte _v): r(_r), s(_s), v(_v) {}
        operator Signature() const { return *(h520 const*)this; }

        /// @returns true if r,s,v values are valid, otherwise false
        bool isValid() const noexcept;
        bool isZero() const;
        bool lowS() const { return s > c_secp256k1n / 2; }
        h256 r;
        h256 s;
        byte v = 0;

        void streamRLP(RLPStream& _s, byte chainID) const {
            _s << (unsigned) (v + (chainID*2 + 35)) << (u256)r << (u256)s;
        }
    };
};

template <class C> typename C::Public toPublic(typename C::Secret const& _secret);
template <class C> typename C::Signature sign(typename C::Secret const& _k, h256 const& _hash);
template <class C> bool verify(typename C::Public const& _k, typename C::Signature const& _s, h256 const& _hash);

template <> typename BLS::Public toPublic<BLS>(typename BLS::Secret const& _secret);
template <> typename ECDSA::Public toPublic<ECDSA>(typename ECDSA::Secret const& _secret);
extern template typename BLS::Public toPublic<BLS>(typename BLS::Secret const& _secret);
extern template typename ECDSA::Public toPublic<ECDSA>(typename ECDSA::Secret const& _secret);

template <> typename BLS::Signature sign<BLS>(typename BLS::Secret const& _k, h256 const& _hash);
template <> typename ECDSA::Signature sign<ECDSA>(typename ECDSA::Secret const& _k, h256 const& _hash);
extern template typename BLS::Signature sign<BLS>(typename BLS::Secret const& _k, h256 const& _hash);
extern template typename ECDSA::Signature sign<ECDSA>(typename ECDSA::Secret const& _k, h256 const& _hash);

template <> bool verify<BLS>(typename BLS::Public const& _k, typename BLS::Signature const& _s, h256 const& _hash);
template <> bool verify<ECDSA>(typename ECDSA::Public const& _k, typename ECDSA::Signature const& _s, h256 const& _hash);
extern template bool verify<BLS>(typename BLS::Public const& _k, typename BLS::Signature const& _s, h256 const& _hash);
extern template bool verify<ECDSA>(typename ECDSA::Public const& _k, typename ECDSA::Signature const& _s, h256 const& _hash);

bool verify(BLS::Public const& _k, BLS::Signature const& _s, h256 const& _hash);
bool verify(ECDSA::Public const& _k, ECDSA::Signature const& _s, h256 const& _hash);

/// Encrypts plain text using Public key.
void encrypt(ECDSA::Public const& _k, bytesConstRef _plain, bytes& o_cipher);

/// Decrypts cipher using Secret key.
bool decrypt(ECDSA::Secret const& _k, bytesConstRef _cipher, bytes& o_plaintext);

/// Encrypt payload using ECIES standard with AES128-CTR.
void encryptECIES(ECDSA::Public const& _k, bytesConstRef _plain, bytes& o_cipher);

/// Encrypt payload using ECIES standard with AES128-CTR.
/// @a _sharedMacData is shared authenticated data.
void encryptECIES(ECDSA::Public const& _k, bytesConstRef _sharedMacData, bytesConstRef _plain, bytes& o_cipher);

/// Decrypt payload using ECIES standard with AES128-CTR.
bool decryptECIES(ECDSA::Secret const& _k, bytesConstRef _cipher, bytes& o_plaintext);

/// Decrypt payload using ECIES standard with AES128-CTR.
/// @a _sharedMacData is shared authenticated data.
bool decryptECIES(ECDSA::Secret const& _k, bytesConstRef _sharedMacData, bytesConstRef _cipher, bytes& o_plaintext);


/// Recovers Public key from signed message hash.
ECDSA::Public recover(ECDSA::Signature const& _sig, h256 const& _hash);
BLS::Public recover(BLS::SignatureStruct const& _sig, h256 const& _hash);

/// A vector of secrets.
//using Secrets = std::vector<Secret>;

/// Convert a public key to address.
template <class C> Address toAddress(typename C::Public const& _public) {
    return right160(sha3(_public.asBytes()));
}

template <class P> Address publicToAddress(P const& _public) {
    return right160(sha3(_public.asBytes()));
}

/// Convert a secret key into address of public key equivalent.
/// @returns 0 if it's not a valid secret key.
template <class C> Address toAddress(typename C::Secret const& _secret) {
    return toAddress<C>(toPublic<C>(_secret));
}

// Convert transaction from and nonce to address.
Address toAddress(Address const& _from, u256 const& _nonce);

/// Symmetric encryption.
void encryptSym(ECDSA::Secret const& _k, bytesConstRef _plain, bytes& o_cipher);

/// Symmetric decryption.
bool decryptSym(ECDSA::Secret const& _k, bytesConstRef _cipher, bytes& o_plaintext);

/// Encrypts payload with random IV/ctr using AES128-CTR.
std::pair<bytes, h128> encryptSymNoAuth(SecureFixedHash<16> const& _k, bytesConstRef _plain);

/// Encrypts payload with specified IV/ctr using AES128-CTR.
bytes encryptAES128CTR(bytesConstRef _k, h128 const& _iv, bytesConstRef _plain);

/// Decrypts payload with specified IV/ctr using AES128-CTR.
bytesSec decryptAES128CTR(bytesConstRef _k, h128 const& _iv, bytesConstRef _cipher);

/// Encrypts payload with specified IV/ctr using AES128-CTR.
inline bytes encryptSymNoAuth(SecureFixedHash<16> const& _k, h128 const& _iv, bytesConstRef _plain) { return encryptAES128CTR(_k.ref(), _iv, _plain); }
inline bytes encryptSymNoAuth(SecureFixedHash<32> const& _k, h128 const& _iv, bytesConstRef _plain) { return encryptAES128CTR(_k.ref(), _iv, _plain); }

/// Decrypts payload with specified IV/ctr using AES128-CTR.
inline bytesSec decryptSymNoAuth(SecureFixedHash<16> const& _k, h128 const& _iv, bytesConstRef _cipher) { return decryptAES128CTR(_k.ref(), _iv, _cipher); }
inline bytesSec decryptSymNoAuth(SecureFixedHash<32> const& _k, h128 const& _iv, bytesConstRef _cipher) { return decryptAES128CTR(_k.ref(), _iv, _cipher); }

/// Derive key via PBKDF2.
bytesSec pbkdf2(std::string const& _pass, bytes const& _salt, unsigned _iterations, unsigned _dkLen = 32);

/// Derive key via Scrypt.
bytesSec scrypt(std::string const& _pass, bytes const& _salt, uint64_t _n, uint32_t _r, uint32_t _p, unsigned _dkLen);

/// Simple class that represents a "key pair".
/// All of the data of the class can be regenerated from the secret key (m_secret) alone.
/// Actually stores a tuplet of secret, public and address (the right 160-bits of the public).

bytes aesDecrypt(bytesConstRef _cipher, std::string const& _password, unsigned _rounds, bytesConstRef _salt);
h256 sha3(bytes const& _input);


template <class C> class KeyPair {
    static const unsigned char addressPrefix = 0xF1;
public:
    typedef typename C::Secret Secret;
    typedef typename C::Public Public;
	/// Normal constructor - populates object from the given secret key.
	/// If the secret key is invalid the constructor succeeds, but public key
    /// and address stay "null".
    KeyPair(Secret const& _sec) :
        m_secret(_sec),
        m_public(toPublic<C>(_sec))
    {
        // Assign address only if the secret key is valid.
        if (m_public)
            m_address = toAddress<C>(m_public);
    }

	/// Create a new, randomly generated object.
    static KeyPair create(bool icap = false, unsigned firstByte = addressPrefix) {
        while (true)
        {
            KeyPair kp(Secret::random());
            while (icap && (kp.address()[0] ^ firstByte)) kp = KeyPair(Secret(sha3(kp.secret().ref())));
            if (kp.address()) return kp;
        }
    }

	/// Create from an encrypted seed.
    static KeyPair fromEncryptedSeed(bytesConstRef _seed, std::string const& _password) {
        return KeyPair(Secret(sha3(aesDecrypt(_seed, _password, 2000, bytesConstRef())).asBytes()));
    }

	Secret const& secret() const { return m_secret; }

	/// Retrieve the public key.
	Public const& pub() const { return m_public; }

	/// Retrieve the associated address of the public key.
	Address const& address() const { return m_address; }

	bool operator==(KeyPair const& _c) const { return m_public == _c.m_public; }
	bool operator!=(KeyPair const& _c) const { return m_public != _c.m_public; }

private:
	Secret m_secret;
	Public m_public;
	Address m_address;
};

extern template class KeyPair<ECDSA>;
extern template class KeyPair<BLS>;

template <class C>
class KeyType
{
public:
    typedef typename C::Secret Secret;
    typedef typename C::Public Public;
    typedef KeyPair<C> Pair;
    typedef typename C::Signature Signature;
    typedef typename C::SignatureStruct SignatureStruct;
    typedef C Type;
    
    static Address toAddress(Public const& p) { return dev::toAddress<C>(p); }
};

typedef KeyType<BLS> AccountKeys;
typedef KeyType<ECDSA> CommKeys;

namespace crypto
{

DEV_SIMPLE_EXCEPTION(InvalidState);

/// Key derivation
h256 kdf(ECDSA::Secret const& _priv, h256 const& _hash);

/**
 * @brief Generator for non-repeating nonce material.
 * The Nonce class should only be used when a non-repeating nonce
 * is required and, in its current form, not recommended for signatures.
 * This is primarily because the key-material for signatures is 
 * encrypted on disk whereas the seed for Nonce is not. 
 * Thus, Nonce's primary intended use at this time is for networking 
 * where the key is also stored in plaintext.
 */
class Nonce
{
public:
	/// Returns the next nonce (might be read from a file).
	static CommKeys::Secret get() { static Nonce s; return s.next(); }

private:
	Nonce() = default;

	/// @returns the next nonce.
	CommKeys::Secret next();

	std::mutex x_value;
	CommKeys::Secret m_value;
};

namespace ecdh
{

bool agree(ECDSA::Secret const& _s, ECDSA::Public const& _r, ECDSA::Secret& o_s) noexcept;

}

namespace ecies
{

bytes kdf(ECDSA::Secret const& _z, bytes const& _s1, unsigned kdByteLen);

}
}
}
