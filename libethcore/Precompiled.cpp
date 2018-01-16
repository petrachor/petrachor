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
/** @file Precompiled.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Precompiled.h"
#include <libdevcore/Log.h>
#include <libdevcore/SHA3.h>
#include <libdevcrypto/Hash.h>
#include <libdevcrypto/Common.h>
#include <libdevcrypto/LibSnark.h>
#include <libethcore/Common.h>
using namespace std;
using namespace dev;
using namespace dev::eth;

PrecompiledRegistrar* PrecompiledRegistrar::s_this = nullptr;

PrecompiledExecutor const& PrecompiledRegistrar::executor(std::string const& _name)
{
	if (!get()->m_execs.count(_name))
		BOOST_THROW_EXCEPTION(ExecutorNotFound());
	return get()->m_execs[_name];
}

PrecompiledPricer const& PrecompiledRegistrar::pricer(std::string const& _name)
{
	if (!get()->m_pricers.count(_name))
		BOOST_THROW_EXCEPTION(PricerNotFound());
	return get()->m_pricers[_name];
}

namespace
{

ETH_REGISTER_PRECOMPILED(ecrecover)(bytesConstRef _in)
{
    struct
    {
        h256 hash;
        h256 v;
        h256 r;
        h256 s;
    } in;

    memcpy(&in, _in.data(), min(_in.size(), sizeof(in)));

    h256 ret;
    u256 v = (u256)in.v;
    if (v >= 27 && v <= 28)
    {
        ECDSA::SignatureStruct sig(in.r, in.s, (byte)((int)v - 27));
        if (sig.isValid())
        {
            try
            {
                if (ECDSA::Public rec = recover(sig, in.hash))
                {
                    ret = dev::sha3(rec);
                    memset(ret.data(), 0, 12);
                    return {true, ret.asBytes()};
                }
            }
            catch (...) {}
        }
    }
    return {true, {}};
}

unsigned long int functionID(const std::string& signature) {
    return *(unsigned long int*) sha3(signature).asBytes().data();
}

bytesConstRef getCroppedBytesConstRef(size_t& updatedOffset, size_t dataSize, bytesConstRef data) {
    size_t orgOffset = updatedOffset;
    updatedOffset += dataSize;
    return data.cropped(orgOffset, updatedOffset);
}

const unsigned char* getCroppedData(size_t& updatedOffset, size_t dataSize, bytesConstRef data) {
    return getCroppedBytesConstRef(updatedOffset, dataSize, data).data();
}

class TupleDecoder {
private:
    size_t offset = 0;
    bytesConstRef data;
public:
   // TupleDecoder() {}
    TupleDecoder(bytesConstRef iData) : data(iData) { }

    Address getAddress() { return *(Address*) getCroppedData(offset, sizeof(h256), data); } // XXX: Possibly not endian-safe
    h256 getH256() { return *(h256*) getCroppedData(offset, sizeof(h256), data); }
    u256 getU256() { return *(u256*) getCroppedData(offset, sizeof(u256), data); }

    bytesConstRef getNextData() {

    }

    bytes getBytes() {
        size_t o = offset; offset += sizeof(h256);
        bytesConstRef bytesOffset = data.cropped(o, offset);
        u256 bytesOffsetI = *(u256*) bytesOffset.data();
        bytesConstRef bytesData = data.cropped((size_t) bytesOffsetI);
        size_t bytesLengthI = *(size_t*) bytesData.data();
        return bytesData.cropped(sizeof(u256), (size_t) bytesLengthI).toBytes();
    }
};

class DynamicArrayDecoder {
private:
    TupleDecoder tupleDecoder;
    u256 k;
public:
    DynamicArrayDecoder(bytesConstRef iData) : tupleDecoder(iData), k(tupleDecoder.getU256()) {}

    bytesConstRef getNextData() { return tupleDecoder.getNextData(); }
};

unsigned long int getFunctionID(bytesConstRef b) {
    return b.size() < 4 ? 0 : (*(unsigned long int*) b.data());
}

template <class G>
class BasicGroup {
public:
    static size_t getSize() { return G::size; }

    static bytes getZero() { return G::getZero().asBytes(); }
    static bytes getOne() { return G::getOne().asBytes(); }

    static std::pair<bool, bytes> executeCommand(bytesConstRef _in) {
        TupleDecoder d(_in.cropped(4));
        const unsigned long int funcID = getFunctionID(_in);
        if (funcID == functionID("getOne()")) { return {true, getOne()}; }
        if (funcID == functionID("getZero()")) { return {true, getZero()}; }
        return {false, bytes()};
    }
};

template <class G, class S>
class AdditiveGroup : public BasicGroup<G> {
public:
    static bytes hashTo(bytes seed, bytes data) { return G::mapToElement(ref(seed), ref(data)).asBytes(); }

    static bytes add(bytes a, bytes b) { return G(ref(a)).add(G(ref(b))).asBytes(); }
    static bytes negate(bytes a) { return G(ref(a)).neg().asBytes(); }

    static bytes scalarMul(bytes g, bytes scalar) { return G(ref(g)).mul(S(scalar)).asBytes(); }

    static std::pair<bool, bytes> executeCommand(bytesConstRef _in) {
        TupleDecoder d(_in.cropped(4));
        const unsigned long int funcID = getFunctionID(_in);
        if (funcID == functionID("hashTo(bytes,bytes)")) { return {true, hashTo(d.getBytes(), d.getBytes())}; }
        if (funcID == functionID("add(bytes,bytes)")) { return {true, add(d.getBytes(), d.getBytes())}; }
        if (funcID == functionID("negate(bytes)")) { return {true, negate(d.getBytes())}; }
        if (funcID == functionID("scalarMul(bytes,bytes)")) { return {true, scalarMul(d.getBytes(), d.getBytes())}; }
        return BasicGroup<G>::executeCommand(_in);
    }
};

template <class G>
class MultiplicativeGroup : public BasicGroup<G> {
public:
    static bytes mul(bytes a, bytes b) { return G(ref(a)).mul(G(ref(b))).asBytes(); }
    static bytes inverse(bytes a) { return G(ref(a)).inv().asBytes(); }

    static std::pair<bool, bytes> executeCommand(bytesConstRef _in) {
        TupleDecoder d(_in.cropped(4));
        const unsigned long int funcID = getFunctionID(_in);
        if (funcID == functionID("mul(bytes,bytes)")) return {true, mul(d.getBytes(), d.getBytes())};
        if (funcID == functionID("inverse(bytes)")) return {true, inverse(d.getBytes())};
        return BasicGroup<G>::executeCommand(_in);
    }
};

ETH_REGISTER_PRECOMPILED(BLS12_381_G1)(bytesConstRef _in) {
    return AdditiveGroup<dev::BLS12_381::G1, dev::BLS12_381::Scalar>::executeCommand(_in);
}

ETH_REGISTER_PRECOMPILED(BLS12_381_G2)(bytesConstRef _in) {
    return AdditiveGroup<dev::BLS12_381::G2, dev::BLS12_381::Scalar>::executeCommand(_in);
}

ETH_REGISTER_PRECOMPILED(BLS12_381_GT)(bytesConstRef _in) {
    return MultiplicativeGroup<dev::BLS12_381::GT>::executeCommand(_in);
}

ETH_REGISTER_PRECOMPILED(BLS12_381_Pairing)(bytesConstRef _in) {
    TupleDecoder d(_in);
    bytes g1 = d.getBytes(), g2 = d.getBytes();
    return {true, dev::BLS12_381::GT::fromPairing(dev::BLS12_381::G1(ref(g1)), dev::BLS12_381::G2(ref(g2))).asBytes() };
}

ETH_REGISTER_PRECOMPILED(BLS12_381_MultiPairing)(bytesConstRef _in) {
    TupleDecoder d(_in);
    bytes g1 = d.getBytes(), g2 = d.getBytes();
    return {true, dev::BLS12_381::GT::fromPairing(dev::BLS12_381::G1(ref(g1)), dev::BLS12_381::G2(ref(g2))).asBytes() };
}

ETH_REGISTER_PRECOMPILED(sha256)(bytesConstRef _in)
{
	return {true, dev::sha256(_in).asBytes()};
}

ETH_REGISTER_PRECOMPILED(ripemd160)(bytesConstRef _in)
{
	return {true, h256(dev::ripemd160(_in), h256::AlignRight).asBytes()};
}

ETH_REGISTER_PRECOMPILED(identity)(bytesConstRef _in)
{
	return {true, _in.toBytes()};
}

// Parse _count bytes of _in starting with _begin offset as big endian int.
// If there's not enough bytes in _in, consider it infinitely right-padded with zeroes.
bigint parseBigEndianRightPadded(bytesConstRef _in, bigint const& _begin, bigint const& _count)
{
	if (_begin > _in.count())
		return 0;
	assert(_count <= numeric_limits<size_t>::max() / 8); // Otherwise, the return value would not fit in the memory.

	size_t const begin{_begin};
	size_t const count{_count};

	// crop _in, not going beyond its size
	bytesConstRef cropped = _in.cropped(begin, min(count, _in.count() - begin));

	bigint ret = fromBigEndian<bigint>(cropped);
	// shift as if we had right-padding zeroes
	assert(count - cropped.count() <= numeric_limits<size_t>::max() / 8);
	ret <<= 8 * (count - cropped.count());

	return ret;
}

ETH_REGISTER_PRECOMPILED(modexp)(bytesConstRef _in)
{
	bigint const baseLength(parseBigEndianRightPadded(_in, 0, 32));
	bigint const expLength(parseBigEndianRightPadded(_in, 32, 32));
	bigint const modLength(parseBigEndianRightPadded(_in, 64, 32));
	assert(modLength <= numeric_limits<size_t>::max() / 8); // Otherwise gas should be too expensive.
	assert(baseLength <= numeric_limits<size_t>::max() / 8); // Otherwise, gas should be too expensive.
	if (modLength == 0 && baseLength == 0)
		return {true, bytes{}}; // This is a special case where expLength can be very big.
	assert(expLength <= numeric_limits<size_t>::max() / 8);

	bigint const base(parseBigEndianRightPadded(_in, 96, baseLength));
	bigint const exp(parseBigEndianRightPadded(_in, 96 + baseLength, expLength));
	bigint const mod(parseBigEndianRightPadded(_in, 96 + baseLength + expLength, modLength));

	bigint const result = mod != 0 ? boost::multiprecision::powm(base, exp, mod) : bigint{0};

	size_t const retLength(modLength);
	bytes ret(retLength);
	toBigEndian(result, ret);

	return {true, ret};
}

namespace
{
	bigint expLengthAdjust(bigint const& _expOffset, bigint const& _expLength, bytesConstRef _in)
	{
		if (_expLength <= 32)
		{
			bigint const exp(parseBigEndianRightPadded(_in, _expOffset, _expLength));
			return exp ? msb(exp) : 0;
		}
		else
		{
			bigint const expFirstWord(parseBigEndianRightPadded(_in, _expOffset, 32));
			size_t const highestBit(expFirstWord ? msb(expFirstWord) : 0);
			return 8 * (_expLength - 32) + highestBit;
		}
	}

	bigint multComplexity(bigint const& _x)
	{
		if (_x <= 64)
			return _x * _x;
		if (_x <= 1024)
			return (_x * _x) / 4 + 96 * _x - 3072;
		else
			return (_x * _x) / 16 + 480 * _x - 199680;
	}
}

ETH_REGISTER_PRECOMPILED_PRICER(modexp)(bytesConstRef _in)
{
	bigint const baseLength(parseBigEndianRightPadded(_in, 0, 32));
	bigint const expLength(parseBigEndianRightPadded(_in, 32, 32));
	bigint const modLength(parseBigEndianRightPadded(_in, 64, 32));

	bigint const maxLength(max(modLength, baseLength));
	bigint const adjustedExpLength(expLengthAdjust(baseLength + 96, expLength, _in));

	return multComplexity(maxLength) * max<bigint>(adjustedExpLength, 1) / 20;
}

ETH_REGISTER_PRECOMPILED(alt_bn128_G1_add)(bytesConstRef _in)
{
	return dev::crypto::alt_bn128_G1_add(_in);
}

ETH_REGISTER_PRECOMPILED(alt_bn128_G1_mul)(bytesConstRef _in)
{
	return dev::crypto::alt_bn128_G1_mul(_in);
}

ETH_REGISTER_PRECOMPILED(alt_bn128_pairing_product)(bytesConstRef _in)
{
	return dev::crypto::alt_bn128_pairing_product(_in);
}

ETH_REGISTER_PRECOMPILED_PRICER(alt_bn128_pairing_product)(bytesConstRef _in)
{
	return 100000 + (_in.size() / 192) * 80000;
}

}
