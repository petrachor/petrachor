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
/** @file Ethash.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * A proof of work algorithm.
 */

#pragma once

#include <thread>

#include <libethcore/SealEngine.h>
#include <libethereum/GenericFarm.h>
#include <libethcore/BlockHeader.h>
#include <libdevcrypto/Common.h>

namespace dev
{

namespace eth
{


class Ethash: public SealEngineBase
{
public:
	std::string name() const override { return "PoS v4"; }
	unsigned revision() const override { return 1; }
    enum { PublicKeyField, StakeModifierField, StakeSignatureField, BlockSignatureField, SealFieldCount };
    unsigned sealFields() const override { return SealFieldCount; }
    bytes sealRLP() const override { return rlp(StakeKeys::Public()) + rlp(Nonce()) + rlp(StakeKeys::Signature()) + rlp(StakeKeys::Signature()); }

	StringHashMap jsInfo(BlockHeader const& _bi) const override;
	void verify(Strictness _s, BlockHeader const& _bi, BalanceRetriever balanceRetriever, BlockHeader const& _parent, bytesConstRef _block) const override;
	void verifyTransaction(ImportRequirements::value _ir, TransactionBase const& _t, BlockHeader const& _header, u256 const& _startGasUsed) const override;
    void populateFromParent(BlockHeader& _bi, BlockHeader const& _parent) override;

    static StakeModifier computeChildStakeModifier(StakeModifier const& parentStakeModifier, StakeKeys::Public const& minterPubKey, StakeKeys::Signature const& minterStakeSig);
    static StakeMessage computeStakeMessage(StakeModifier const& modifier, u256 timestamp);
    static StakeKeys::Signature computeStakeSignature(StakeMessage const& message, StakeKeys::Secret const& sealerSecretKey);
    static bool verifyStakeSignature(StakeKeys::Public const& publicKey, StakeKeys::Signature const& signature, StakeMessage const& message);
    static StakeSignatureHash computeStakeSignatureHash(StakeKeys::Signature const& stakeSignature);

	strings sealers() const override;
	std::string sealer() const override { return m_sealer; }
	void setSealer(std::string const& _sealer) override { m_sealer = _sealer; }
    void cancelGeneration() override { m_generating = false; }
    void generateSeal(BlockHeader _bi, BlockHeader const& parent, BalanceRetriever balanceRetriever) override;
	bool shouldSeal(Interface* _i) override;

    static StakeKeys::Public publicKey(BlockHeader const& _bi) { return _bi.seal<StakeKeys::Public>(PublicKeyField); }
    static StakeKeys::Signature stakeSignature(BlockHeader const& _bi) { return _bi.seal<StakeKeys::Signature>(StakeSignatureField); }
    static StakeKeys::Signature blockSignature(BlockHeader const& _bi) { return _bi.seal<StakeKeys::Signature>(BlockSignatureField); }
    static StakeModifier stakeModifier(BlockHeader const& _bi) { return _bi.seal<StakeModifier>(StakeModifierField); }
    h256 boundary(BlockHeader const& _bi, u256 const& balance) const;
    static BlockHeader& setPublicKey(BlockHeader& _bi, StakeKeys::Public _v) { _bi.setSeal(PublicKeyField, _v); return _bi; }
    static BlockHeader& setStakeSignature(BlockHeader& _bi, StakeKeys::Signature _v) { _bi.setSeal(StakeSignatureField, _v); return _bi; }
    static BlockHeader& setBlockSignature(BlockHeader& _bi, StakeKeys::Signature _v) { _bi.setSeal(BlockSignatureField, _v); return _bi; }
    static BlockHeader& setStakeModifier(BlockHeader& _bi, StakeModifier _v) { _bi.setSeal(StakeModifierField, _v); return _bi; }

    u256 calculateDifficulty(BlockHeader const& _bi, bigint const& parentTimeStamp, bigint const& parentDifficulty) const;
    u256 calculateDifficulty(BlockHeader const& _bi, BlockHeader const& parent) const;
    u256 childGasLimit(BlockHeader const& _bi, u256 const& _gasFloorTarget = Invalid256) const;

	void manuallySetWork(BlockHeader const& _work) { m_sealing = _work; }

	static void init();

    bool isMining() const { return m_generating; }
private:
    u256 getAgedBalance(Address a, BlockNumber bn, BalanceRetriever balanceRetriever) const;
    bool verifySeal(BlockHeader const& _bi, BlockHeader const& m_parent, BalanceRetriever balanceRetriever) const;

	std::string m_sealer = "cpu";
    BlockHeader m_sealing;

    //StakeModifier m_parentStakeModifier;
    u256 minimalTimeStamp(BlockHeader const& parent) { return parent.timestamp() + 1; }

    bool m_generating = false;
    std::thread sealThread;
	/// A mutex covering m_sealing
    Mutex m_submitLock, m_sealThreadLock;
};

}
}
