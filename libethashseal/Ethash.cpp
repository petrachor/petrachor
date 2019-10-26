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
/**
 *  This is where Proof of Stake is carried out.
 *  The Algorithm is a mixture of Proof of Work and Proof of Stake
 *  If verification passes it would send the block to the network
 *  It uses public key that avoid no-node user getting involved.
 * 
 * 
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

/**
 *  computerChildStakeModifier is a Ethash member function.
 *  It hashes its parameters using dev::sha3 function.
 *  
 *  @param parentStakeModifier(const reference): Its type is StakeModifier.
 *  @param minerPubKey(const reference): Its type is StakeKeys::Public.
 *  @param minterStakeSig(const reference): Its type is StakeKeys::Signature.
 *  @returns StakeModifier which is a hash.
 */
StakeModifier Ethash::computeChildStakeModifier(StakeModifier const& parentStakeModifier, StakeKeys::Public const& minerPubKey, StakeKeys::Signature const& minterStakeSig) {
    return dev::sha3(parentStakeModifier.asBytes() + minerPubKey.asBytes() + minterStakeSig.asBytes());
}

/**
 *  computeStatkeMessage is a Ethash member function. 
 *  It hashes its parameters using dev::sha3 function.
 * 
 *  @param stakeModifier(const reference): Its type is StakeModifier.
 *  @param timestamp: Its type is u256.
 *  @returns StakeMessage which is a hash.
 */
StakeMessage Ethash::computeStakeMessage(StakeModifier const& stakeModifier, u256 timestamp) {
    return dev::sha3(stakeModifier.asBytes() + h256(timestamp));
}

/**
 *  computeStakeSignature is a Ethash member function. 
 *  It signs Stake message using its secret key.
 * 
 *  @param message(const reference): Its type is StakeMessage.
 *  @param sealerSecretKey(const reference): Its type is StakeKeys::Secret.
 *  @returns StakeKeys::Signature.
 */
StakeKeys::Signature Ethash::computeStakeSignature(StakeMessage const& message, StakeKeys::Secret const& sealerSecretKey) {
    return sign<dev::BLS>(sealerSecretKey, message);
}

/**
 *  verifyStakeSignature is a Ethash member function. 
 *  It verifies Stake message.
 * 
 *  @param publicKey(const reference): Its type is StakeKeys::Public.
 *  @param signature(const reference): Its type is StakeKeys::Signature.
 *  @param message(const reference): Its type is StakeMessage.
 *  @returns bool which indicates if verification passed or not.
 */
bool Ethash::verifyStakeSignature(StakeKeys::Public const& publicKey, StakeKeys::Signature const& signature, StakeMessage const& message) {
    return ::verify<BLS>(publicKey, signature, message);
}

/**
 *  computeStakeSignatureHash is a Ethash member function.
 *  It is a wrapper over dev::sha3. 
 * 
 *  @param stakeSignature(const reference): Its type is StakeKeys::Signature.
 *  @returns StakeSignatureHash which is sha3 hash.
 */
StakeSignatureHash Ethash::computeStakeSignatureHash(StakeKeys::Signature const& stakeSignature) {
    return dev::sha3(stakeSignature);
}

strings Ethash::sealers() const
{
	return {"cpu"};
}

/**
 *  jsInfo is a Ethash member function. 
 *  
 *  @param _bi(const reference): Its type is BlockHeader. This is the block under consideration.
 *  @returns StringHashMap. 
 */
StringHashMap Ethash::jsInfo(BlockHeader const& _bi) const
{
    return { { "stakeModifier", toJS(stakeModifier(_bi)) }, { "publicKey", toJS(publicKey(_bi)) }, { "stakeSignature", toJS(stakeSignature(_bi)) }, { "blockSignature", toJS(blockSignature(_bi)) },
         { "difficulty", toJS(_bi.difficulty()) } };
}

/**
 *  verify is a Ethash member function. 
 * 
 *  @param _s: Its type is Strictness.  
 *  @param _bi(const reference): Its type is BlockHeader. This is the block under consideration.
 *  @param balanceRetriever: Its type is BalanceRetriever. This is a function object.
 *  @param _parent(const reference): Its type is BlockHeader. This is the tip of the blockchain.
 *  @param _block: Its type is bytesConstRef.
 *  @returns void.
 */
void Ethash::verify(Strictness _s, BlockHeader const& _bi, BalanceRetriever balanceRetriever, BlockHeader const& _parent, bytesConstRef _block) const
{
	SealEngineFace::verify(_s, _bi, balanceRetriever, _parent, _block);

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
        //clog << "Difficulty: " << difficulty << " expected: " << expected << " Parent.number: " << _parent.number() << "\n";
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
    if ((_s == CheckEverything || _s == QuickNonce) && _bi.parentHash() && !verifySeal(_bi, _parent, balanceRetriever))
	{
		InvalidBlockNonce ex;
		ex << errinfo_hash256(_bi.hash(WithoutSeal));
		ex << errinfo_difficulty(_bi.difficulty());
		BOOST_THROW_EXCEPTION(ex);
	}

}

/**
 *  verifyTransaction is a Ethash member function. 
 * 
 *  @param _ir: Its type is ImportRequirements::value.
 *  @param _t(const reference): Its type is TransactionBase. 
 *  @param _header(const reference): Its type is BlockHeader.
 *  @param _startGasUsed(const reference): Its type is u256.
 *  @returns void.
 */
void Ethash::verifyTransaction(ImportRequirements::value _ir, TransactionBase const& _t, BlockHeader const& _header, u256 const& _startGasUsed) const
{
	SealEngineFace::verifyTransaction(_ir, _t, _header, _startGasUsed);

	if (_ir & ImportRequirements::TransactionBasic && _t.baseGasRequired(evmSchedule(_header.number())) > _t.gas())
		BOOST_THROW_EXCEPTION(OutOfGasIntrinsic() << RequirementError((bigint)(_t.baseGasRequired(evmSchedule(_header.number()))), (bigint)_t.gas()));

	// Avoid transactions that would take us beyond the block gas limit.
	if (_startGasUsed + (bigint)_t.gas() > _header.gasLimit())
		BOOST_THROW_EXCEPTION(BlockGasLimitReached() << RequirementError((bigint)(_header.gasLimit() - _startGasUsed), (bigint)_t.gas()));
}

/**
 *  childGasLimit is a Etash member function. 
 * 
 *  @param _bi(const reference): Its type is BlockHeader. This is the block under consideration.
 *  @param _gasFloorTarget(const reference): Its type is u256.
 *  @returns const u256 type.
 */
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

/**
 *  calculateDifficulty is a Ethash member function. 
 * 
 *  @param _bi(const reference): Its type is BlockHeader. This is the block under consideration.
 *  @param _parent(const reference): Its type is BlockHeader. This is the tip of the blockchain.  
 */
u256 Ethash::calculateDifficulty(BlockHeader const& _bi, BlockHeader const& parent) const {
    //clog << "Parent.number: " << parent.number() << " ";
    return calculateDifficulty(_bi, parent.timestamp(), parent.difficulty());
}

/**
 *  calculateDifficulty is a Ethash member function. 
 * 
 *  @param _bi(const reference): Its type is BlockHeader. This is the block under consideration.
 *  @param parentTimeStamp(const reference): Its type is bigint.  
 *  @param parentDifficulty(const reference): Its type is bigint.  
 *  @returns const u256 type.
 */
u256 Ethash::calculateDifficulty(BlockHeader const& _bi, bigint const& parentTimeStamp, bigint const& parentDifficulty) const
{
	if (!_bi.number())
		throw GenesisBlockCannotBeCalculated();

    bigint const timestampDiff = bigint(_bi.timestamp()) - bigint(parentTimeStamp);
    bigint const adjFactor = max<bigint>((chainParams().targetBlockInterval - timestampDiff), -99*128);
    bigint target = parentDifficulty + (parentDifficulty * adjFactor) / chainParams().difficultyBoundDivisor;
    //clog << "Parent timestamp: " << parentTimeStamp << " bi.timestamp " << _bi.timestamp() << " timestampDiff: " << timestampDiff << " parent diff: " << parentDifficulty << " target: " << target << "\n";
    return u256(min<bigint>(max<bigint>(chainParams().minimumDifficulty, target), std::numeric_limits<u256>::max()));
}

/**
 *  populateFromParent is a member function. 
 * 
 *  @param _bi(const reference): Its type is BlockHeader. This is the block under consideration.   
 *  @param _parent(const reference): Its type is BlockHeader. This is the tip of the blockchain. 
 */
void Ethash::populateFromParent(BlockHeader& _bi, BlockHeader const& _parent)
{
	SealEngineFace::populateFromParent(_bi, _parent);
    _bi.setGasLimit(childGasLimit(_parent));
}

/**
 *  boundary is a Ethash member function. 
 * 
 *  @param _bi(const reference): Its type is BlockHeader. This is the block under consideration.
 *  @param balance(const reference): Its type is u256.   
 *  @returns const h256.
 */
h256 Ethash::boundary(BlockHeader const& _bi, u256 const& balance) const {
    auto d = _bi.difficulty();
   // clog << "balance: " << balance << "\n";
    return d ? (h256)u256(((bigint(1) << 256)/ d) * balance) : h256();
}

/**
 *  getAgedBalance is a Ethash member function. 
 * 
 *  @param a: Its type is Address.
 *  @param balanceRetriever: Its type is BalanceRetriever. This is a function object.   
 */
u256 Ethash::getAgedBalance(Address a, BlockNumber bn, BalanceRetriever balanceRetriever) const {
    return balanceRetriever(a, (bn > 255) ? (bn - 255) : (BlockNumber) 0);
}

/**
 *  verifySeal is a Ethash member function. 
 * 
 *  @param _bi(const reference): Its type is BlockHeader. This is the block under consideration.  
 *  @param _parent(const reference): Its type is BlockHeader. This is the tip of the blockchain.
 *  @param balanceRetriever: Its type is BalanceRetriever. This is a function object. 
 *  @returns const bool that tell if Seal verification passed or not. 
 */
bool Ethash::verifySeal(BlockHeader const& _bi, BlockHeader const& _parent, BalanceRetriever balanceRetriever) const
{
    const StakeKeys::Signature stakeSig = stakeSignature(_bi);
    bool blockSignatureVerified = ::verify<BLS>(publicKey(_bi), blockSignature(_bi), _bi.hash(WithoutSeal));
    if (_parent) { 
        if (_bi.number() != _parent.number() + 1)
            return false;
        const Address minterAddress = publicToAddress<BLS::Public>(publicKey(_bi));
        const u256 minterBalance = getAgedBalance(minterAddress, (BlockNumber) _parent.number(), balanceRetriever);
        bool meetsBounds = computeStakeSignatureHash(stakeSig) <= boundary(_bi, minterBalance);
        bool modifierCorrect = stakeModifier(_bi) == computeChildStakeModifier(stakeModifier(_parent), publicKey(_bi), stakeSig);
        bool stakeSignatureVerified = verifyStakeSignature(publicKey(_bi), stakeSig, computeStakeMessage(stakeModifier(_parent), _bi.timestamp()));
        if (!(meetsBounds && modifierCorrect && blockSignatureVerified && stakeSignatureVerified)) {
            clog << "verifySeal: " << "meetsBounds: " << meetsBounds << " modifierCorrect: " << modifierCorrect << " blockSignatureVerified: " 
                 << blockSignatureVerified << " stakeSignatureVerified: " << stakeSignatureVerified << "\n";
            clog << "stakeSig: " << stakeSig.hex() 
                 << "stakeMessage: " << computeStakeMessage(stakeModifier(_parent), _bi.timestamp()).hex()
                 << " modifier: " << stakeModifier(_bi).hex()
                 << " expected: " << computeChildStakeModifier(stakeModifier(_parent), publicKey(_bi), stakeSig).hex() << "\n";
        }
        return meetsBounds && modifierCorrect && blockSignatureVerified && stakeSignatureVerified;
    } else {
        return blockSignatureVerified;
    }
}

/**
 *  generateSeal is a Ethash member function. 
 *  It  generates and send Seal to the network if it is properly verified.
 *  
 *  @param _bi(const reference): Its type is BlockHeader. This is the block under consideration.  
 *  @param _parent(const reference): Its type is BlockHeader. This is the tip of the blockchain.
 *  @param balanceRetriever: Its type is BalanceRetriever. This is a function object. 
 *  @returns void. 
 */
/*
void Ethash::generateSeal(BlockHeader _bi, BlockHeader const& parent, BalanceRetriever balanceRetriever)
{
    clog << " generate seal for " << _bi.number() << " m_parent.number: " << parent.number() << "\n";
    if (!m_generating || !m_sealing || (m_sealing.hash(WithoutSeal) != _bi.hash(WithoutSeal))) {
        Guard l(m_submitLock);
        if (sealThread.joinable()) sealThread.join();
        m_sealing = _bi;
        m_generating = true;
        sealThread = std::thread([balanceRetriever, parent, this](){
            u256 timestamp = minimalTimeStamp(parent);
            //      clog << "Minimal timestamp: " << timestamp << "\n";
            //std::map<Address, u256> balanceMap;
            //for (auto kp: m_keyPairs) balanceMap.insert(std::make_pair(kp.address(), getAgedBalance(kp.address(), (BlockNumber) parent.number(), balanceRetriever)));
            //This While loop is confusing...it has no clear exit.
            if(m_generating) {
                u256 currentTime;
                //while (m_generating && (timestamp > (currentTime = utcTime()))) this_thread::sleep_for(chrono::milliseconds(20));
                //if (!m_generating) break;

                //currentTime = utcTime();
                //m_sealing.setTimestamp(currentTime);
                //m_sealing.setDifficulty(calculateDifficulty(m_sealing, parent));
                for (auto kp: m_keyPairs) {
                    //u256 balance = balanceMap.find(kp.address())->second;
                    u256 balance = getAgedBalance(kp.address(), (BlockNumber) parent.number(), balanceRetriever);
                    if (balance != (u256) 0) {
                        currentTime = utcTime();
                        m_sealing.setTimestamp(currentTime);
                        m_sealing.setDifficulty(calculateDifficulty(m_sealing, parent));
                        //     clog << "[parent ts: " << parent.timestamp() << " ts: " << timestamp << " delta:" << (timestamp - parent.timestamp()) << " kp: " << m_keyPairs.size() << " p: " << parent.number() << " d: " << calculateDifficulty(m_sealing, parent) << " b: " << boundary(m_sealing, balance).hex() << "]";
                        const StakeKeys::Signature r = computeStakeSignature(computeStakeMessage(stakeModifier(parent), timestamp), kp.secret());
                        if (computeStakeSignatureHash(r) <= boundary(m_sealing, balance)) {
                            std::unique_lock<Mutex> l(m_submitLock);
                            setStakeModifier(m_sealing, computeChildStakeModifier(stakeModifier(parent), kp.pub(), r));
                            setPublicKey(m_sealing, kp.pub());
                            setStakeSignature(m_sealing, r);
                            setBlockSignature(m_sealing, sign<BLS>(kp.secret(), m_sealing.hash(WithoutSeal)));

                            if (m_onSealGenerated && verifySeal(m_sealing, parent, balanceRetriever))
                            {
                                //assert(verifySeal(m_sealing, parent, balanceRetriever));

                                RLPStream ret;
                                m_sealing.streamRLP(ret);
                                l.unlock();
                                m_onSealGenerated(ret.out());
                            }
                            m_generating  = false;
                            return;
                        }
                    }
                }
                //timestamp += 1;
          //      if (m_generating && lastPaused) this_thread::sleep_for(chrono::milliseconds(500));
            }
            return;
        });
    }
}
*/

void Ethash::generateSeal(BlockHeader _bi, BlockHeader const& parent, BalanceRetriever balanceRetriever)
{
    clog << " generate seal for " << _bi.number() << " m_parent.number: " << parent.number() << "\n";
    Guard l(m_submitLock);
    //if (sealThread.joinable()) sealThread.join();
    m_sealing = _bi;
    //m_generating = true;
    u256 tempBalance = (u256)0;
    u256 balance;
    KeyPair<BLS> keyPair = KeyPair<BLS>::create(); 
    // This is hacky, no need of key in the first place.
    for (auto kp: m_keyPairs) {
        balance = getAgedBalance(kp.address(), (BlockNumber) parent.number(), balanceRetriever);
        if (balance > tempBalance) {
            tempBalance = balance;
            keyPair = kp;
        }
    }
    balance = tempBalance;
    auto kp = keyPair;
    if (balance != (u256) 0) {
        std::thread sealThread([&](){
            //u256 timestamp = minimalTimeStamp(parent);
            u256 timestamp = max<u256>(utcTime(), minimalTimeStamp(parent));
            while (minimalTimeStamp(parent) > utcTime()) this_thread::sleep_for(chrono::milliseconds(20));
            
            m_sealing.setTimestamp(timestamp);
            m_sealing.setDifficulty(calculateDifficulty(m_sealing, parent));
            clog << "[parent ts: " << parent.timestamp() << " ts: " << timestamp << " delta:" << (timestamp - parent.timestamp()) << " kp: " << m_keyPairs.size() << " p: " << parent.number() << " d: " << calculateDifficulty(m_sealing, parent) << " b: " << boundary(m_sealing, balance).hex() << "]";
            const StakeKeys::Signature r = computeStakeSignature(computeStakeMessage(stakeModifier(parent), timestamp), kp.secret());
            if (computeStakeSignatureHash(r) <= boundary(m_sealing, balance)) {
                std::unique_lock<Mutex> l(m_submitLock);
                setStakeModifier(m_sealing, computeChildStakeModifier(stakeModifier(parent), kp.pub(), r));
                setPublicKey(m_sealing, kp.pub());
                setStakeSignature(m_sealing, r);
                setBlockSignature(m_sealing, sign<BLS>(kp.secret(), m_sealing.hash(WithoutSeal)));

                if (m_onSealGenerated && verifySeal(m_sealing, parent, balanceRetriever))
                {
                    //assert(verifySeal(m_sealing, parent, balanceRetriever));
                    RLPStream ret;
                    m_sealing.streamRLP(ret);
                    l.unlock();
                    m_onSealGenerated(ret.out());
                }
                //m_generating  = false;
                return;
            }                
        });
    }
    return;

}

/**
 *  shouldSeal is a Ethash member function. 
 *  It does nothing.
 * 
 *  @returns bool valu true.
 */
bool Ethash::shouldSeal(Interface*)
{
	return true;
}
