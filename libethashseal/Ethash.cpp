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

StakeModifier Ethash::computeChildStakeModifier(StakeModifier const& parentStakeModifier, Address const& sealerAddress) {
    bytes d(parentStakeModifier.size + sealerAddress.size);

    size_t offset = 0;
    for (size_t x = 0; x < parentStakeModifier.size; ++x) d[offset++] = parentStakeModifier[x];
    for (size_t x = 0; x < sealerAddress.size; ++x) d[offset++] = sealerAddress[x];

    return dev::sha3(d);
}

StakeMessage Ethash::computeStakeMessage(StakeModifier const& stakeModifier, u64 timestamp) {
    bytes d(stakeModifier.size + sizeof(timestamp));

    size_t offset = 0;
    for (size_t x = 0; x < stakeModifier.size; ++x) d[offset++] = stakeModifier[x];
    for (size_t x = 0; x < sizeof(timestamp); ++x) d[offset++] = ((unsigned char*) &timestamp)[x];

    return dev::sha3(d);
}

StakeSignature Ethash::computeStakeSignature(StakeMessage const& message, Secret const& sealerSecretKey) {
    return StakeSignature(sign<dev::BLS>(sealerSecretKey, message), toPublic<BLS>(sealerSecretKey));
}

bool Ethash::verifyStakeSignature(StakeSignature const& signature, StakeMessage const& message) {
    return ::verify<BLS>(signature.publicKey, signature, message);
}

StakeSignatureHash Ethash::computeStakeSignatureHash(StakeSignature const& stakeSignature) {
    return dev::sha3(stakeSignature);
}

strings Ethash::sealers() const
{
	return {"cpu"};
}

h256 Ethash::seedHash(BlockHeader const& _bi)
{
    return computeChildStakeModifier(_bi.stakeModifier(), _bi.author());
}

StringHashMap Ethash::jsInfo(BlockHeader const& _bi) const
{
    return { { "nonce", toJS(nonce(_bi)) },// { "seedHash", toJS(seedHash(_bi)) },
        { "boundary", toJS(boundary(_bi)) }, { "difficulty", toJS(_bi.difficulty()) } };
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
    if ((_s == CheckEverything || _s == QuickNonce) && _bi.parentHash() && !verifySeal(_bi))
	{
		InvalidBlockNonce ex;
		ex << errinfo_nonce(nonce(_bi));
//		ex << errinfo_mixHash(mixHash(_bi));
//		ex << errinfo_seedHash(seedHash(_bi));
//		EthashProofOfWork::Result er = EthashAux::eval(seedHash(_bi), _bi.hash(WithoutSeal), nonce(_bi));
//		ex << errinfo_ethashResult(make_tuple(er.value, er.mixHash));
		ex << errinfo_hash256(_bi.hash(WithoutSeal));
		ex << errinfo_difficulty(_bi.difficulty());
		ex << errinfo_target(boundary(_bi));
		BOOST_THROW_EXCEPTION(ex);
	}
/*	else if (_s == QuickNonce && _bi.parentHash() && !quickVerifySeal(_bi))
	{
		InvalidBlockNonce ex;
		ex << errinfo_hash256(_bi.hash(WithoutSeal));
		ex << errinfo_difficulty(_bi.difficulty());
		ex << errinfo_nonce(nonce(_bi));
		BOOST_THROW_EXCEPTION(ex);
    }*/
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

u256 Ethash::calculateDifficulty(BlockHeader const& _bi, BlockHeader const& _parent) const
{
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

h256 Ethash::boundary(BlockHeader const& _bi) const {
    auto d = _bi.difficulty();
    BlockNumber blockNumber = BlockNumber(_bi.number() - 1);
    auto moneySupplyBeforeBlock = getMoneySupplyAtBlock(blockNumber);
    auto authorBalance = m_balanceRetriever(_bi.author(), blockNumber);
    return d ? (h256)u256((((bigint(1) << 256) / moneySupplyBeforeBlock) * authorBalance)/ d) : h256();
}

bool Ethash::verifySeal(BlockHeader const& _bi) const
{
    StakeSignature sig = signature(_bi);
    return computeStakeSignatureHash(sig) <= boundary(_bi)
            && verifyStakeSignature(sig, computeStakeMessage(seedHash(_bi), nonce(_bi)));
}

void Ethash::generateSeal(BlockHeader _bi)
{
    // PubKey + Statehash + Stakemod
    m_generating = true;
    Guard l(m_submitLock);
    m_sealing = _bi;
    sealThread = std::thread([&](){
        while (m_generating) {
            uint64_t timestamp = utcTime();
            Nonce n = (Nonce) timestamp;
            assert(sizeof(n) == sizeof(timestamp));
            for (auto kp: m_keyPairs) {
                //u256 balance = m_balanceRetriever(kp.address(), (BlockNumber) (_bi.number() - 1));
                _bi.setAuthor(kp.address());
                //_bi.setTimestamp((u256) timestamp);
                StakeModifier modifier = seedHash(_bi);
                StakeSignature r = computeStakeSignature(computeStakeMessage(modifier, n), kp.secret());
                if (computeStakeSignatureHash(r) <= boundary(m_sealing)) {
                    std::unique_lock<Mutex> l(m_submitLock);
            //         cdebug << m_farm.work().seedHash << m_farm.work().headerHash << sol.nonce << EthashAux::eval(m_farm.work().seedHash, m_farm.work().headerHash, sol.nonce).value;
                    setSignature(m_sealing, r);
                    setNonce(m_sealing, n);

                    if (m_onSealGenerated)
                    {
                        RLPStream ret;
                        m_sealing.streamRLP(ret);
                        l.unlock();
                        m_onSealGenerated(ret.out());
                    }
                    return true;
                };
            }
            usleep(500);
        }
        return true;
    });
}

bool Ethash::shouldSeal(Interface*)
{
	return true;
}
