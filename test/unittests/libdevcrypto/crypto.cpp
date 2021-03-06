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
/** @file crypto.cpp
 * @author Alex Leverington <nessence@gmail.com>
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Crypto test functions.
 */

#include <libdevcore/Guards.h>
#include <secp256k1.h>
#include <cryptopp/keccak.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/sha.h>
#include <cryptopp/modes.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/osrng.h>
#include <cryptopp/oids.h>
#include <cryptopp/dsa.h>
#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/Log.h>
#include <boost/test/unit_test.hpp>
#include <libdevcore/SHA3.h>
#include <libdevcrypto/Hash.h>
#include <libdevcrypto/CryptoPP.h>
#include <libethereum/Transaction.h>
#include <test/tools/libtesteth/TestOutputHelper.h>
#include <test/tools/libtesteth/Options.h>

using namespace std;
using namespace dev;
using namespace dev::test;
using namespace dev::crypto;

typedef KeyPair<ECDSA> Keys;

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(Crypto)

struct DevcryptoTestFixture: public TestOutputHelper {
	DevcryptoTestFixture() : s_secp256k1(Secp256k1PP::get()) {}

	Secp256k1PP* s_secp256k1;
};
BOOST_FIXTURE_TEST_SUITE(devcrypto, DevcryptoTestFixture)

static CryptoPP::AutoSeededRandomPool& rng()
{
	static CryptoPP::AutoSeededRandomPool s_rng;
	return s_rng;
}

static CryptoPP::OID& curveOID()
{
	static CryptoPP::OID s_curveOID(CryptoPP::ASN1::secp256k1());
	return s_curveOID;
}

static CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP>& params()
{
	static CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> s_params(curveOID());
	return s_params;
}

BOOST_AUTO_TEST_CASE(sha3general)
{
	BOOST_REQUIRE_EQUAL(sha3(""), h256("c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470"));
	BOOST_REQUIRE_EQUAL(sha3("hello"), h256("1c8aff950685c2ed4bc3174f3472287b56d9517b9c948127319a09a7a36deac8"));
}

BOOST_AUTO_TEST_CASE(emptySHA3Types)
{
	h256 emptySHA3(fromHex("c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470"));
	BOOST_REQUIRE_EQUAL(emptySHA3, EmptySHA3);

	h256 emptyListSHA3(fromHex("1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347"));
	BOOST_REQUIRE_EQUAL(emptyListSHA3, EmptyListSHA3);
}

BOOST_AUTO_TEST_CASE(pubkeyOfZero)
{
    auto pub = toPublic<BLS>({});
	BOOST_REQUIRE_EQUAL(pub, Public{});
}

BOOST_AUTO_TEST_CASE(KeyPairMix)
{
    Keys k = Keys::create();
	BOOST_REQUIRE(!!k.secret());
	BOOST_REQUIRE(!!k.pub());
    auto test = toPublic<Keys::Type>(k.secret());
	BOOST_CHECK_EQUAL(k.pub(), test);
}

BOOST_AUTO_TEST_CASE(KeyPairVerifySecret)
{
    auto keyPair = Keys::create();
	auto* ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
	BOOST_CHECK(secp256k1_ec_seckey_verify(ctx, keyPair.secret().data()));
	secp256k1_context_destroy(ctx);
}


BOOST_AUTO_TEST_CASE(SignAndRecoverLoop)
{
	auto num = 13;
	auto msg = h256::random();
	while (--num)
	{
		msg = sha3(msg);
        auto kp = Keys::create();
        auto sig = sign<Keys::Type>(kp.secret(), msg);
		BOOST_CHECK(verify(kp.pub(), sig, msg));
		auto pub = recover(sig, msg);
		BOOST_CHECK_EQUAL(kp.pub(), pub);
	}
}

BOOST_AUTO_TEST_CASE(cryptopp_patch)
{
    Keys k = Keys::create();
	bytes io_text;
	s_secp256k1->decrypt(k.secret(), io_text);
	BOOST_REQUIRE_EQUAL(io_text.size(), 0);
}

BOOST_AUTO_TEST_CASE(verify_secert_bls)
{
    Secret empty;
    Keys kNot(empty);
    BOOST_REQUIRE(!kNot.address());
    Keys k(sha3(empty));
    BOOST_REQUIRE(k.address());
}

BOOST_AUTO_TEST_CASE(verify_secert_ecdsa)
{
    Secret empty;
    KeyPair<ECDSA> kNot(empty);
    BOOST_REQUIRE(!kNot.address());
    KeyPair<ECDSA> k(sha3(empty));
    BOOST_REQUIRE(k.address());
}

BOOST_AUTO_TEST_CASE(common_encrypt_decrypt)
{
	string message("Now is the time for all good persons to come to the aid of humanity.");
	bytes m = asBytes(message);
	bytesConstRef bcr(&m);

    Keys k = Keys::create();
	bytes cipher;
	encrypt(k.pub(), bcr, cipher);
	BOOST_REQUIRE(cipher != asBytes(message) && cipher.size() > 0);
	
	bytes plain;
	decrypt(k.secret(), bytesConstRef(&cipher), plain);
	
	BOOST_REQUIRE(asString(plain) == message);
	BOOST_REQUIRE(plain == asBytes(message));
}

BOOST_AUTO_TEST_CASE(sha3_norestart)
{
	CryptoPP::Keccak_256 ctx;
	bytes input(asBytes("test"));
	ctx.Update(input.data(), 4);
	CryptoPP::Keccak_256 ctxCopy(ctx);
	bytes interimDigest(32);
	ctx.Final(interimDigest.data());
	ctx.Update(input.data(), 4);
	bytes firstDigest(32);
	ctx.Final(firstDigest.data());
	BOOST_REQUIRE(interimDigest == firstDigest);
	
	ctxCopy.Update(input.data(), 4);
	bytes finalDigest(32);
	ctxCopy.Final(interimDigest.data());
	BOOST_REQUIRE(interimDigest != finalDigest);
	
	// we can do this another way -- copy the context for final
	ctxCopy.Update(input.data(), 4);
	ctxCopy.Update(input.data(), 4);
	CryptoPP::Keccak_256 finalCtx(ctxCopy);
	bytes finalDigest2(32);
	finalCtx.Final(finalDigest2.data());
	BOOST_REQUIRE(finalDigest2 == interimDigest);
	ctxCopy.Update(input.data(), 4);
	bytes finalDigest3(32);
	finalCtx.Final(finalDigest3.data());
	BOOST_REQUIRE(finalDigest2 != finalDigest3);
}

BOOST_AUTO_TEST_CASE(ecies_kdf)
{
    Keys local = Keys::create(), remote = Keys::create();
	// nonce
    Keys::Secret z1;
	BOOST_CHECK(ecdh::agree(local.secret(), remote.pub(), z1));
	auto key1 = ecies::kdf(z1, bytes(), 64);
	bytesConstRef eKey1 = bytesConstRef(&key1).cropped(0, 32);
	bytesRef mKey1 = bytesRef(&key1).cropped(32, 32);
	sha3(mKey1, mKey1);
	
    Keys::Secret z2;
	BOOST_CHECK(ecdh::agree(remote.secret(), local.pub(), z2));
	auto key2 = ecies::kdf(z2, bytes(), 64);
	bytesConstRef eKey2 = bytesConstRef(&key2).cropped(0, 32);
	bytesRef mKey2 = bytesRef(&key2).cropped(32, 32);
	sha3(mKey2, mKey2);
	
	BOOST_REQUIRE(eKey1.toBytes() == eKey2.toBytes());
	BOOST_REQUIRE(mKey1.toBytes() == mKey2.toBytes());
	
	BOOST_REQUIRE(!!z1);
	BOOST_REQUIRE(z1 == z2);
	
	BOOST_REQUIRE(key1.size() > 0 && ((u512)h512(key1)) > 0);
	BOOST_REQUIRE(key1 == key2);
}

BOOST_AUTO_TEST_CASE(ecdh_agree_invalid_pubkey)
{
    Keys ok = Keys::create();
    Keys::Public pubkey;
	~pubkey;  // Create a pubkey of all 1s.
    BLS::Secret z;
	BOOST_CHECK(!ecdh::agree(ok.secret(), pubkey, z));
}

BOOST_AUTO_TEST_CASE(ecdh_agree_invalid_seckey)
{
    Keys ok = Keys::create();
    Keys::Secret seckey;  // "Null" seckey is invalid.
	BOOST_CHECK(!ecdh::agree(seckey, ok.pub(), seckey));
}

BOOST_AUTO_TEST_CASE(ecies_standard)
{
    Keys k = Keys::create();
	
	string message("Now is the time for all good persons to come to the aid of humanity.");
	string original = message;
	bytes b = asBytes(message);
	
	s_secp256k1->encryptECIES(k.pub(), b);
	BOOST_REQUIRE(b != asBytes(original));
	BOOST_REQUIRE(b.size() > 0 && b[0] == 0x04);
	
	s_secp256k1->decryptECIES(k.secret(), b);
	BOOST_REQUIRE(bytesConstRef(&b).cropped(0, original.size()).toBytes() == asBytes(original));
}

BOOST_AUTO_TEST_CASE(ecies_sharedMacData)
{
    Keys k = Keys::create();

	string message("Now is the time for all good persons to come to the aid of humanity.");
	bytes original = asBytes(message);
	bytes b = original;

	string shared("shared MAC data");
	string wrongShared("wrong shared MAC data");

	s_secp256k1->encryptECIES(k.pub(), shared, b);
	BOOST_REQUIRE(b != original);
	BOOST_REQUIRE(b.size() > 0 && b[0] == 0x04);

	BOOST_REQUIRE(!s_secp256k1->decryptECIES(k.secret(), wrongShared, b));

	s_secp256k1->decryptECIES(k.secret(), shared, b);

	auto decrypted = bytesConstRef(&b).cropped(0, original.size()).toBytes();
	BOOST_CHECK_EQUAL(toHex(decrypted), toHex(original));
}

BOOST_AUTO_TEST_CASE(ecies_eckeypair)
{
    Keys k = Keys::create();

	string message("Now is the time for all good persons to come to the aid of humanity.");
	string original = message;
	
	bytes b = asBytes(message);
	s_secp256k1->encrypt(k.pub(), b);
	BOOST_REQUIRE(b != asBytes(original));

	s_secp256k1->decrypt(k.secret(), b);
	BOOST_REQUIRE(b == asBytes(original));
}

BOOST_AUTO_TEST_CASE(ecdhCryptopp)
{
	CryptoPP::ECDH<CryptoPP::ECP>::Domain dhLocal(curveOID());
	CryptoPP::SecByteBlock privLocal(dhLocal.PrivateKeyLength());
	CryptoPP::SecByteBlock pubLocal(dhLocal.PublicKeyLength());
	dhLocal.GenerateKeyPair(rng(), privLocal, pubLocal);

	CryptoPP::ECDH<CryptoPP::ECP>::Domain dhRemote(curveOID());
	CryptoPP::SecByteBlock privRemote(dhRemote.PrivateKeyLength());
	CryptoPP::SecByteBlock pubRemote(dhRemote.PublicKeyLength());
	dhRemote.GenerateKeyPair(rng(), privRemote, pubRemote);

	assert(dhLocal.AgreedValueLength() == dhRemote.AgreedValueLength());

	// local: send public to remote; remote: send public to local

	// Local
	CryptoPP::SecByteBlock sharedLocal(dhLocal.AgreedValueLength());
	assert(dhLocal.Agree(sharedLocal, privLocal, pubRemote));
	
	// Remote
	CryptoPP::SecByteBlock sharedRemote(dhRemote.AgreedValueLength());
	assert(dhRemote.Agree(sharedRemote, privRemote, pubLocal));
	
	// Test
	CryptoPP::Integer ssLocal, ssRemote;
	ssLocal.Decode(sharedLocal.BytePtr(), sharedLocal.SizeInBytes());
	ssRemote.Decode(sharedRemote.BytePtr(), sharedRemote.SizeInBytes());
	
	assert(ssLocal != 0);
	assert(ssLocal == ssRemote);
	
	
	// Now use our keys
    KeyPair<ECDSA> a = KeyPair<ECDSA>::create();
	byte puba[65] = {0x04};
	memcpy(&puba[1], a.pub().data(), 64);
	
    KeyPair<ECDSA> b = KeyPair<ECDSA>::create();
	byte pubb[65] = {0x04};
	memcpy(&pubb[1], b.pub().data(), 64);

	CryptoPP::ECDH<CryptoPP::ECP>::Domain dhA(curveOID());
	Secret shared;
	BOOST_REQUIRE(dhA.Agree(shared.writable().data(), a.secret().data(), pubb));
	BOOST_REQUIRE(shared);
}

BOOST_AUTO_TEST_CASE(ecdhe)
{
    auto local = Keys::create();
    auto remote = Keys::create();
	BOOST_CHECK_NE(local.pub(), remote.pub());

	// local tx pubkey -> remote
	Secret sremote;
	BOOST_CHECK(ecdh::agree(remote.secret(), local.pub(), sremote));
	
	// remote tx pbukey -> local
	Secret slocal;
	BOOST_CHECK(ecdh::agree(local.secret(), remote.pub(), slocal));

	BOOST_CHECK(sremote);
	BOOST_CHECK(slocal);
	BOOST_CHECK_EQUAL(sremote, slocal);
}

/*
BOOST_AUTO_TEST_CASE(handshakeNew)
{
	//	authInitiator -> E(remote-pubk, S(ecdhe-random, ecdh-shared-secret^nonce) || H(ecdhe-random-pubk) || pubk || nonce || 0x0)
	//	authRecipient -> E(remote-pubk, ecdhe-random-pubk || nonce || 0x0)
	
	h256 base(sha3("privacy"));
	sha3(base.ref(), base.ref());
    BLS::Secret nodeAsecret(base);
    Keys nodeA(nodeAsecret);
	BOOST_REQUIRE(nodeA.pub());
	
	sha3(base.ref(), base.ref());
    BLS::Secret nodeBsecret(base);
    Keys nodeB(nodeBsecret);
	BOOST_REQUIRE(nodeB.pub());
	
	BOOST_REQUIRE_NE(nodeA.secret(), nodeB.secret());
	
	// Initiator is Alice (nodeA)
    auto eA = Keys::create();
	bytes nAbytes(fromHex("0xAAAA"));
	h256 nonceA(sha3(nAbytes));
    const size_t sigSize = BLS::SignatureStruct::size;
    const size_t publicSize = BLS::Public::size;
    bytes auth(sigSize + h256::size + Public::size + h256::size + 1);
    BLS::Secret ssA;
	{
        bytesRef sig(&auth[0], sigSize);
        bytesRef hepubk(&auth[sigSize], h256::size);
        bytesRef pubk(&auth[sigSize + h256::size], publicSize);
        bytesRef nonce(&auth[sigSize + h256::size + publicSize], h256::size);
		
		BOOST_CHECK(crypto::ecdh::agree(nodeA.secret(), nodeB.pub(), ssA));
        sign<ECDSA>(eA.secret(), (ssA ^ nonceA).makeInsecure()).ref().copyTo(sig);
		sha3(eA.pub().ref(), hepubk);
		nodeA.pub().ref().copyTo(pubk);
		nonceA.ref().copyTo(nonce);
		auth[auth.size() - 1] = 0x0;
	}
	bytes authcipher;
	encrypt(nodeB.pub(), &auth, authcipher);
	BOOST_REQUIRE_EQUAL(authcipher.size(), 279);
	
	// Receipient is Bob (nodeB)
    auto eB = Keys::create();
	bytes nBbytes(fromHex("0xBBBB"));
	h256 nonceB(sha3(nAbytes));
    bytes ack(BLS::Public::size + h256::size + 1);
	{
		// todo: replace nodeA.pub() in encrypt()
		// decrypt public key from auth
		bytes authdecrypted;
		decrypt(nodeB.secret(), &authcipher, authdecrypted);
        BLS::Public node;
        bytesConstRef pubk(&authdecrypted[BLS::Signature::size + h256::size], BLS::Public::size);
		pubk.copyTo(node.ref());
		
        bytesRef epubk(&ack[0], BLS::Public::size);
        bytesRef nonce(&ack[BLS::Public::size], h256::size);

		eB.pub().ref().copyTo(epubk);
		nonceB.ref().copyTo(nonce);
		auth[auth.size() - 1] = 0x0;
	}
	bytes ackcipher;
	encrypt(nodeA.pub(), &ack, ackcipher);
	BOOST_REQUIRE_EQUAL(ackcipher.size(), 182);
	
	BOOST_REQUIRE(eA.pub());
	BOOST_REQUIRE(eB.pub());
	BOOST_REQUIRE_NE(eA.secret(), eB.secret());
	
	/// Alice (after receiving ack)
    BLS::Secret aEncryptK;
    BLS::Secret aMacK;
    BLS::Secret aEgressMac;
    BLS::Secret aIngressMac;
	{
		bytes ackdecrypted;
		decrypt(nodeA.secret(), &ackcipher, ackdecrypted);
		BOOST_REQUIRE(ackdecrypted.size());
		bytesConstRef ackRef(&ackdecrypted);
        BLS::Public eBAck;
		h256 nonceBAck;
        ackRef.cropped(0, Public::size).copyTo(bytesRef(eBAck.data(), BLS::Public::size));
		ackRef.cropped(Public::size, h256::size).copyTo(nonceBAck.ref());
		BOOST_REQUIRE_EQUAL(eBAck, eB.pub());
		BOOST_REQUIRE_EQUAL(nonceBAck, nonceB);
		
		// TODO: export ess and require equal to b
		
		bytes keyMaterialBytes(512);
		bytesRef keyMaterial(&keyMaterialBytes);
		
        BLS::Secret ess;
		// todo: ecdh-agree should be able to output bytes
		BOOST_CHECK(ecdh::agree(eA.secret(), eBAck, ess));
		ess.ref().copyTo(keyMaterial.cropped(0, h256::size));
		ssA.ref().copyTo(keyMaterial.cropped(h256::size, h256::size));
//		auto token = sha3(ssA);
		aEncryptK = sha3Secure(keyMaterial);
		aEncryptK.ref().copyTo(keyMaterial.cropped(h256::size, h256::size));
		aMacK = sha3Secure(keyMaterial);

		keyMaterialBytes.resize(h256::size + authcipher.size());
		keyMaterial.retarget(keyMaterialBytes.data(), keyMaterialBytes.size());
		(aMacK ^ nonceBAck).ref().copyTo(keyMaterial);
		bytesConstRef(&authcipher).copyTo(keyMaterial.cropped(h256::size, authcipher.size()));
		aEgressMac = sha3Secure(keyMaterial);
		
		keyMaterialBytes.resize(h256::size + ackcipher.size());
		keyMaterial.retarget(keyMaterialBytes.data(), keyMaterialBytes.size());
		(aMacK ^ nonceA).ref().copyTo(keyMaterial);
		bytesConstRef(&ackcipher).copyTo(keyMaterial.cropped(h256::size, ackcipher.size()));
		aIngressMac = sha3Secure(keyMaterial);
	}
	
	
	/// Bob (after sending ack)
    BLS::Secret ssB;
	BOOST_CHECK(crypto::ecdh::agree(nodeB.secret(), nodeA.pub(), ssB));
	BOOST_REQUIRE_EQUAL(ssA, ssB);
	
    BLS::Secret bEncryptK;
    BLS::Secret bMacK;
    BLS::Secret bEgressMac;
    BLS::Secret bIngressMac;
	{
		bytes authdecrypted;
		decrypt(nodeB.secret(), &authcipher, authdecrypted);
		BOOST_REQUIRE(authdecrypted.size());
		bytesConstRef ackRef(&authdecrypted);
        BLS::Signature sigAuth;
		h256 heA;
        BLS::Public eAAuth;
        BLS::Public nodeAAuth;
        BLS::SignatureStruct authSS(sigAuth, nodeAAuth);
        const size_t sigSize = sizeof(BLS::SignatureStruct);
        typedef BLS::Secret Nonce;
        Nonce nonceAAuth;
        bytesConstRef sig(&authdecrypted[0], sigSize);
        bytesConstRef hepubk(&authdecrypted[sigSize], h256::size);
        bytesConstRef pubk(&authdecrypted[sigSize + h256::size], BLS::Public::size);
        bytesConstRef nonce(&authdecrypted[sigSize + h256::size + BLS::Public::size], Nonce::size);

        nonce.makeInsecure().copyTo(nonceAAuth.ref());
		pubk.copyTo(nodeAAuth.ref());
		BOOST_REQUIRE(nonceAAuth);
		BOOST_REQUIRE_EQUAL(nonceA, nonceAAuth);
		BOOST_REQUIRE(nodeAAuth);
		BOOST_REQUIRE_EQUAL(nodeA.pub(), nodeAAuth); // bad test, bad!!!
		hepubk.copyTo(heA.ref());
		sig.copyTo(sigAuth.ref());
		
        SignatureStruct ss(sigAuth, nodeAAuth);
        BLS::Secret sec(sigAuth, nodeAAuth);
		BOOST_CHECK(ecdh::agree(nodeB.secret(), nodeAAuth, ss));
        eAAuth = recover(ss, (sec ^ nonceAAuth).makeInsecure());
		// todo: test when this fails; means remote is bad or packet bits were flipped
		BOOST_REQUIRE_EQUAL(heA, sha3(eAAuth));
		BOOST_REQUIRE_EQUAL(eAAuth, eA.pub());
		
		bytes keyMaterialBytes(512);
		bytesRef keyMaterial(&keyMaterialBytes);
		
        BLS::Secret ess;
		// todo: ecdh-agree should be able to output bytes
		BOOST_CHECK(ecdh::agree(eB.secret(), eAAuth, ess));
//		s_secp256k1->agree(eB.seckey(), eAAuth, ess);
		ess.ref().copyTo(keyMaterial.cropped(0, h256::size));
		ssB.ref().copyTo(keyMaterial.cropped(h256::size, h256::size));
//		auto token = sha3(ssA);
		bEncryptK = sha3Secure(keyMaterial);
		bEncryptK.ref().copyTo(keyMaterial.cropped(h256::size, h256::size));
		bMacK = sha3Secure(keyMaterial);
		
		// todo: replace nonceB with decrypted nonceB
		keyMaterialBytes.resize(h256::size + ackcipher.size());
		keyMaterial.retarget(keyMaterialBytes.data(), keyMaterialBytes.size());
		(bMacK ^ nonceAAuth).ref().copyTo(keyMaterial);
		bytesConstRef(&ackcipher).copyTo(keyMaterial.cropped(h256::size, ackcipher.size()));
		bEgressMac = sha3Secure(keyMaterial);
		
		keyMaterialBytes.resize(h256::size + authcipher.size());
		keyMaterial.retarget(keyMaterialBytes.data(), keyMaterialBytes.size());
		(bMacK ^ nonceB).ref().copyTo(keyMaterial);
		bytesConstRef(&authcipher).copyTo(keyMaterial.cropped(h256::size, authcipher.size()));
		bIngressMac = sha3Secure(keyMaterial);
	}
	
	BOOST_REQUIRE_EQUAL(aEncryptK, bEncryptK);
	BOOST_REQUIRE_EQUAL(aMacK, bMacK);
	BOOST_REQUIRE_EQUAL(aEgressMac, bIngressMac);
	BOOST_REQUIRE_EQUAL(bEgressMac, aIngressMac);
	
	
	
}
*/
BOOST_AUTO_TEST_CASE(ecies_aes128_ctr_unaligned)
{
	SecureFixedHash<16> encryptK(sha3("..."), h128::AlignLeft);
	h256 egressMac(sha3("+++"));
	// TESTING: send encrypt magic sequence
	bytes magic {0x22,0x40,0x08,0x91};
	bytes magicCipherAndMac;
	magicCipherAndMac = encryptSymNoAuth(encryptK, h128(), &magic);
	
	magicCipherAndMac.resize(magicCipherAndMac.size() + 32);
	sha3mac(egressMac.ref(), &magic, egressMac.ref());
	egressMac.ref().copyTo(bytesRef(&magicCipherAndMac).cropped(magicCipherAndMac.size() - 32, 32));
	
	bytesConstRef cipher(&magicCipherAndMac[0], magicCipherAndMac.size() - 32);
	bytes plaintext = decryptSymNoAuth(encryptK, h128(), cipher).makeInsecure();
	
	plaintext.resize(magic.size());
	// @alex @subtly TODO: FIX: this check is pointless with the above line.
	BOOST_REQUIRE(plaintext.size() > 0);
	BOOST_REQUIRE(magic == plaintext);
}

BOOST_AUTO_TEST_CASE(ecies_aes128_ctr)
{
	SecureFixedHash<16> k(sha3("0xAAAA"), h128::AlignLeft);
	string m = "AAAAAAAAAAAAAAAA";
	bytesConstRef msg((byte*)m.data(), m.size());

	bytes ciphertext;
	h128 iv;
	tie(ciphertext, iv) = encryptSymNoAuth(k, msg);
	
	bytes plaintext = decryptSymNoAuth(k, iv, &ciphertext).makeInsecure();
	BOOST_REQUIRE_EQUAL(asString(plaintext), m);
}

BOOST_AUTO_TEST_CASE(cryptopp_aes128_ctr)
{
	const int aesKeyLen = 16;
	BOOST_REQUIRE(sizeof(char) == sizeof(byte));
	
	// generate test key
	CryptoPP::AutoSeededRandomPool rng;
	CryptoPP::SecByteBlock key(0x00, aesKeyLen);
	rng.GenerateBlock(key, key.size());
	
	// cryptopp uses IV as nonce/counter which is same as using nonce w/0 ctr
	FixedHash<CryptoPP::AES::BLOCKSIZE> ctr;
	rng.GenerateBlock(ctr.data(), sizeof(ctr));

	// used for decrypt
	FixedHash<CryptoPP::AES::BLOCKSIZE> ctrcopy(ctr);
	
	string text = "Now is the time for all good persons to come to the aid of humanity.";
	unsigned char const* in = (unsigned char*)&text[0];
	unsigned char* out = (unsigned char*)&text[0];
	string original = text;
	string doublespeak = text + text;
	
	string cipherCopy;
	try
	{
		CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption e;
		e.SetKeyWithIV(key, key.size(), ctr.data());
		
		// 68 % 255 should be difference of counter
		e.ProcessData(out, in, text.size());
		ctr = h128(u128(ctr) + text.size() / 16);
		
		BOOST_REQUIRE(text != original);
		cipherCopy = text;
	}
	catch (CryptoPP::Exception& _e)
	{
		cerr << _e.what() << endl;
	}
	
	try
	{
		CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption d;
		d.SetKeyWithIV(key, key.size(), ctrcopy.data());
		d.ProcessData(out, in, text.size());
		BOOST_REQUIRE(text == original);
	}
	catch (CryptoPP::Exception& _e)
	{
		cerr << _e.what() << endl;
	}
	
	
	// reencrypt ciphertext...
	try
	{
		BOOST_REQUIRE(cipherCopy != text);
		in = (unsigned char*)&cipherCopy[0];
		out = (unsigned char*)&cipherCopy[0];

		CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption e;
		e.SetKeyWithIV(key, key.size(), ctrcopy.data());
		e.ProcessData(out, in, text.size());
		
		// yep, ctr mode.
		BOOST_REQUIRE(cipherCopy == original);
	}
	catch (CryptoPP::Exception& _e)
	{
		cerr << _e.what() << endl;
	}
	
}

BOOST_AUTO_TEST_CASE(cryptopp_aes128_cbc)
{
	const int aesKeyLen = 16;
	BOOST_REQUIRE(sizeof(char) == sizeof(byte));

	CryptoPP::AutoSeededRandomPool rng;
	CryptoPP::SecByteBlock key(0x00, aesKeyLen);
	rng.GenerateBlock(key, key.size());
	
	// Generate random IV
	byte iv[CryptoPP::AES::BLOCKSIZE];
	rng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);
	
	string string128("AAAAAAAAAAAAAAAA");
	string plainOriginal = string128;
	
	CryptoPP::CBC_Mode<CryptoPP::Rijndael>::Encryption cbcEncryption(key, key.size(), iv);
	cbcEncryption.ProcessData((byte*)&string128[0], (byte*)&string128[0], string128.size());
	BOOST_REQUIRE(string128 != plainOriginal);

	CryptoPP::CBC_Mode<CryptoPP::Rijndael>::Decryption cbcDecryption(key, key.size(), iv);
	cbcDecryption.ProcessData((byte*)&string128[0], (byte*)&string128[0], string128.size());
	BOOST_REQUIRE(plainOriginal == string128);
	
	
	// plaintext whose size isn't divisible by block size must use stream filter for padding
	string string192("AAAAAAAAAAAAAAAABBBBBBBB");
	plainOriginal = string192;

	string cipher;
	CryptoPP::StreamTransformationFilter* aesStream = new CryptoPP::StreamTransformationFilter(cbcEncryption, new CryptoPP::StringSink(cipher));
	CryptoPP::StringSource source(string192, true, aesStream);
	BOOST_REQUIRE(cipher.size() == 32);

	byte* pOut = reinterpret_cast<byte*>(&string192[0]);
	byte const* pIn = reinterpret_cast<byte const*>(cipher.data());
	cbcDecryption.ProcessData(pOut, pIn, cipher.size());
	BOOST_REQUIRE(string192 == plainOriginal);
}

BOOST_AUTO_TEST_CASE(recoverVgt3)
{
	// base secret
	Secret secret(sha3("privacy"));

	// we get ec params from signer
	CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::Keccak_256>::Signer signer;

	// e := sha3(msg)
	bytes e(fromHex("0x01"));
	e.resize(32);
	int tests = 13;
	while (sha3(&e, &e), secret = sha3(secret), tests--)
	{
        KeyPair<ECDSA> key(secret);
        ECDSA::Public pkey = key.pub();
		signer.AccessKey().Initialize(params(), CryptoPP::Integer(secret.data(), Secret::size));

		h256 he(sha3(e));
		CryptoPP::Integer heInt(he.asBytes().data(), 32);
		h256 k(crypto::kdf(secret, he));
		CryptoPP::Integer kInt(k.asBytes().data(), 32);
		kInt %= params().GetSubgroupOrder()-1;

		CryptoPP::ECP::Point rp = params().ExponentiateBase(kInt);
		CryptoPP::Integer const& q = params().GetGroupOrder();
		CryptoPP::Integer r = params().ConvertElementToInteger(rp);

		CryptoPP::Integer kInv = kInt.InverseMod(q);
		CryptoPP::Integer s = (kInv * (CryptoPP::Integer(secret.data(), 32) * r + heInt)) % q;
		BOOST_REQUIRE(!!r && !!s);

		//try recover function on diffrent v values (should be invalid)
		for (size_t i = 0; i < 10; i++)
		{
            ECDSA::Signature sig;
			sig[64] = i;
			r.Encode(sig.data(), 32);
			s.Encode(sig.data() + 32, 32);

            ECDSA::Public p = dev::recover(sig, he);
			size_t expectI = rp.y.IsOdd() ? 1 : 0;
			if (i == expectI)
				BOOST_REQUIRE(p == pkey);
			else
				BOOST_REQUIRE(p != pkey);
		}
	}
}

BOOST_AUTO_TEST_CASE(PerfSHA256_32, *utf::label("perf"))
{
	if (!test::Options::get().all)
	{
		std::cout << "Skipping test Crypto/devcrypto/PerfSHA256_32. Use --all to run it.\n";
		return;
	}

	h256 hash;
	for (auto i = 0; i < 1000000; ++i)
		hash = sha256(hash.ref());

	BOOST_CHECK_EQUAL(hash[0], 0x2a);
}

BOOST_AUTO_TEST_CASE(PerfSHA256_4000, *utf::label("perf"))
{
	if (!test::Options::get().all)
	{
		std::cout << "Skipping test Crypto/devcrypto/PerfSHA256_4000. Use --all to run it.\n";
		return;
	}

	static const size_t dataSize = 4097;
	bytes data(dataSize);
	for (auto i = 0; i < 100000; ++i)
	{
		auto hash = sha256(&data);
		auto idx = ((hash[1] << 8) | hash[2]) % (dataSize - hash.size);
		std::copy(hash.data(), hash.data() + hash.size, data.begin() + idx);
	}

	BOOST_CHECK_EQUAL(data[0], 0x4d);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

