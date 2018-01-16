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
/** @file Ethash.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Ethash.h"
#include <libethereum/Interface.h>
#include <libethcore/ChainOperationParams.h>
#include <libethcore/CommonJS.h>

using namespace std;
using namespace dev;
using namespace eth;

void Ethash::init()
{
	ETH_REGISTER_SEAL_ENGINE(Ethash);
}

StakeModifier Ethash::computeChildStakeModifier(StakeModifier const& parentStakeModifier, Public const& sealerPublicKey) {
    bytes d(parentStakeModifier.size + sealerPublicKey.size);

    size_t offset = 0;
    for (size_t x = 0; x < parentStakeModifier.size; ++x) d[offset++] = parentStakeModifier[x];
    for (size_t x = 0; x < sealerPublicKey.size; ++x) d[offset++] = sealerPublicKey[x];

    return dev::sha3(d);
}

StakeHash Ethash::computeStakeHash(StakeModifier const& seedHash, u64 timestamp, Secret const& sealerSecretKey) {
    bytes d(seedHash.size + sizeof(timestamp));

    size_t offset = 0;
    for (size_t x = 0; x < seedHash.size; ++x) d[offset++] = seedHash[x];
    for (size_t x = 0; x < sizeof(timestamp); ++x) d[offset++] = ((unsigned char*) &timestamp)[x];

    return sign(sealerSecretKey, dev::sha3(d));
}

strings Ethash::sealers() const
{
	return {"cpu"};
}

h256 Ethash::seedHash(BlockHeader const& _bi, Public const& sealerPublicKey)
{
    return computeChildStakeModifier(_bi.stakeModifier(), sealerPublicKey);
}

StringHashMap Ethash::jsInfo(BlockHeader const& _bi) const
{
    return { { "nonce", toJS(nonce(_bi)) }, { "seedHash", toJS(seedHash(_bi)) }, { "boundary", toJS(boundary(_bi)) }, { "difficulty", toJS(_bi.difficulty()) } };
}

void Ethash::verify(Strictness _s, BlockHeader const& _bi, BlockHeader const& _parent, bytesConstRef _block) const
{
	SealEngineFace::verify(_s, _bi, _parent, _block);

	if (_s != CheckNothingNew)
	{
		if (_bi.difficulty() < chainParams().minimumDifficulty)
			BOOST_THROW_EXCEPTION(InvalidDifficulty() << RequirementError(bigint(chainParams().minimumDifficulty), bigint(_bi.difficulty())) );

		if (_bi.gasLimit() < chainParams().minGasLimit)
			BOOST_THROW_EXCEPTION(InvalidGasLimit() << RequirementError(bigint(chainParams().minGasLimit), bigint(_bi.gasLimit())) );

		if (_bi.gasLimit() > chainParams().maxGasLimit)
			BOOST_THROW_EXCEPTION(InvalidGasLimit() << RequirementError(bigint(chainParams().maxGasLimit), bigint(_bi.gasLimit())) );

		if (_bi.number() && _bi.extraData().size() > chainParams().maximumExtraDataSize)
			BOOST_THROW_EXCEPTION(ExtraDataTooBig() << RequirementError(bigint(chainParams().maximumExtraDataSize), bigint(_bi.extraData().size())) << errinfo_extraData(_bi.extraData()));
	}

	if (_parent)
	{
		// Check difficulty is correct given the two timestamps.
		auto expected = calculateDifficulty(_bi, _parent);
		auto difficulty = _bi.difficulty();
		if (difficulty != expected)
			BOOST_THROW_EXCEPTION(InvalidDifficulty() << RequirementError((bigint)expected, (bigint)difficulty));

		auto gasLimit = _bi.gasLimit();
		auto parentGasLimit = _parent.gasLimit();
		if (
			gasLimit < chainParams().minGasLimit ||
			gasLimit > chainParams().maxGasLimit ||
			gasLimit <= parentGasLimit - parentGasLimit / chainParams().gasLimitBoundDivisor ||
			gasLimit >= parentGasLimit + parentGasLimit / chainParams().gasLimitBoundDivisor)
			BOOST_THROW_EXCEPTION(
				InvalidGasLimit()
				<< errinfo_min((bigint)((bigint)parentGasLimit - (bigint)(parentGasLimit / chainParams().gasLimitBoundDivisor)))
				<< errinfo_got((bigint)gasLimit)
				<< errinfo_max((bigint)((bigint)parentGasLimit + parentGasLimit / chainParams().gasLimitBoundDivisor))
			);
	}

	// check it hashes according to proof of work or that it's the genesis block.
	if (_s == CheckEverything && _bi.parentHash() && !verifySeal(_bi))
	{
		InvalidBlockNonce ex;
		ex << errinfo_nonce(nonce(_bi));
		ex << errinfo_mixHash(mixHash(_bi));
		ex << errinfo_seedHash(seedHash(_bi));
		EthashProofOfWork::Result er = EthashAux::eval(seedHash(_bi), _bi.hash(WithoutSeal), nonce(_bi));
		ex << errinfo_ethashResult(make_tuple(er.value, er.mixHash));
		ex << errinfo_hash256(_bi.hash(WithoutSeal));
		ex << errinfo_difficulty(_bi.difficulty());
		ex << errinfo_target(boundary(_bi));
		BOOST_THROW_EXCEPTION(ex);
	}
	else if (_s == QuickNonce && _bi.parentHash() && !quickVerifySeal(_bi))
	{
		InvalidBlockNonce ex;
		ex << errinfo_hash256(_bi.hash(WithoutSeal));
		ex << errinfo_difficulty(_bi.difficulty());
		ex << errinfo_nonce(nonce(_bi));
		BOOST_THROW_EXCEPTION(ex);
	}
}

void Ethash::verifyTransaction(ImportRequirements::value _ir, TransactionBase const& _t, BlockHeader const& _header, u256 const& _startGasUsed) const
{
	SealEngineFace::verifyTransaction(_ir, _t, _header, _startGasUsed);

	if (_ir & ImportRequirements::TransactionSignatures)
	{
        int chainID = chainParams().chainID;
        _t.checkChainId(chainID);
	}
	if (_ir & ImportRequirements::TransactionBasic && _t.baseGasRequired(evmSchedule(_header.number())) > _t.gas())
		BOOST_THROW_EXCEPTION(OutOfGasIntrinsic() << RequirementError((bigint)(_t.baseGasRequired(evmSchedule(_header.number()))), (bigint)_t.gas()));

	// Avoid transactions that would take us beyond the block gas limit.
	if (_startGasUsed + (bigint)_t.gas() > _header.gasLimit())
		BOOST_THROW_EXCEPTION(BlockGasLimitReached() << RequirementError((bigint)(_header.gasLimit() - _startGasUsed), (bigint)_t.gas()));
}

u256 Ethash::childGasLimit(BlockHeader const& _bi, u256 const& _gasFloorTarget) const
{
	u256 gasFloorTarget = _gasFloorTarget == Invalid256 ? 3141562 : _gasFloorTarget;
	u256 gasLimit = _bi.gasLimit();
	u256 boundDivisor = chainParams().gasLimitBoundDivisor;
	if (gasLimit < gasFloorTarget)
		return min<u256>(gasFloorTarget, gasLimit + gasLimit / boundDivisor - 1);
	else
		return max<u256>(gasFloorTarget, gasLimit - gasLimit / boundDivisor + 1 + (_bi.gasUsed() * 6 / 5) / boundDivisor);
}

void Ethash::manuallySubmitWork(const h256& _mixHash, Nonce _nonce)
{
	m_farm.submitProof(EthashProofOfWork::Solution{_nonce, _mixHash}, nullptr);
}

u256 Ethash::calculateDifficulty(BlockHeader const& _bi, BlockHeader const& _parent) const
{
	const unsigned c_expDiffPeriod = 100000;

	if (!_bi.number())
		throw GenesisBlockCannotBeCalculated();
	auto const& minimumDifficulty = chainParams().minimumDifficulty;

    bigint const timestampDiff = bigint(_bi.timestamp()) - _parent.timestamp();
    bigint const adjFactor = _bi.number() < chainParams().byzantiumForkBlock ?
        max<bigint>(1 - timestampDiff / 10, -99) : // Homestead-era difficulty adjustment
        max<bigint>((_parent.hasUncles() ? 2 : 1) - timestampDiff / 9, -99); // Byzantium-era difficulty adjustment

    bigint target = _parent.difficulty() + _parent.difficulty() / 2048 * adjFactor;
    bigint o = max<bigint>(minimumDifficulty, target);
	return u256(min<bigint>(o, std::numeric_limits<u256>::max()));
}

void Ethash::populateFromParent(BlockHeader& _bi, BlockHeader const& _parent) const
{
	SealEngineFace::populateFromParent(_bi, _parent);
	_bi.setDifficulty(calculateDifficulty(_bi, _parent));
//    _bi.setStakeModifier(computeChildStakeModifier(_parent.stakeModifier(), ));  miner-dependent
	_bi.setGasLimit(childGasLimit(_parent));
}

bool Ethash::verifySeal(BlockHeader const& _bi) const
{
    return eval(seedHash(_bi), nonce(_bi)).value <= boundary(_bi) && result.mixHash == mixHash(_bi);
}

void Ethash::generateSeal(BlockHeader const& _bi)
{
    // PubKey + Statehash + Stakemod
    m_generating = true;
    Guard l(m_submitLock);
    m_sealing = _bi;
    sealThread = std::thread([&](){
        while (m_generating) {
            uint64_t timestamp = utcTime();

            Nonce n;
            assert(sizeof(n) == sizeof(timestamp));
            memcpy(&n, &timestamp, sizeof(n));
            h256 r = computeStakeHash(w.seedHash, n);
            if (r <= boundary(m_sealing)) {
                std::unique_lock<Mutex> l(m_submitLock);
        //         cdebug << m_farm.work().seedHash << m_farm.work().headerHash << sol.nonce << EthashAux::eval(m_farm.work().seedHash, m_farm.work().headerHash, sol.nonce).value;
                setMixHash(m_sealing, sol.mixHash);
                setNonce(m_sealing, sol.nonce);
                if (!quickVerifySeal(m_sealing))
                    return false;

                if (m_onSealGenerated)
                {
                    RLPStream ret;
                    m_sealing.streamRLP(ret);
                    l.unlock();
                    m_onSealGenerated(ret.out());
                }
                return true;
            };

        usleep(500);
        }
    });
}

bool Ethash::shouldSeal(Interface*)
{
	return true;
}
