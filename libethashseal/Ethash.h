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

typedef BLS::SignatureStruct StakeSignature;

class Ethash: public SealEngineBase
{
public:
	std::string name() const override { return "PoS v4"; }
	unsigned revision() const override { return 1; }
	unsigned sealFields() const override { return 2; }
    bytes sealRLP() const override { return rlp(StakeSignature()) + rlp(Nonce()); }

	StringHashMap jsInfo(BlockHeader const& _bi) const override;
	void verify(Strictness _s, BlockHeader const& _bi, BlockHeader const& _parent, bytesConstRef _block) const override;
	void verifyTransaction(ImportRequirements::value _ir, TransactionBase const& _t, BlockHeader const& _header, u256 const& _startGasUsed) const override;
	void populateFromParent(BlockHeader& _bi, BlockHeader const& _parent) const override;

    static StakeModifier computeChildStakeModifier(StakeModifier const& parentStakeModifier, Address const& sealerAddress);
    static StakeMessage computeStakeMessage(StakeModifier const& modifier, u64 timestamp);
    static StakeSignature computeStakeSignature(StakeMessage const& message, Secret const& sealerSecretKey);
    static bool verifyStakeSignature(StakeSignature const& signature, StakeMessage const& message);
    static StakeSignatureHash computeStakeSignatureHash(StakeSignature const& stakeSignature);

	strings sealers() const override;
	std::string sealer() const override { return m_sealer; }
	void setSealer(std::string const& _sealer) override { m_sealer = _sealer; }
    void cancelGeneration() override { m_generating = false; }
    void generateSeal(BlockHeader _bi) override;
	bool shouldSeal(Interface* _i) override;

    enum { SignatureField = 0, NonceField = 1 };
    static h256 seedHash(BlockHeader const& _bi);
    static StakeSignature signature(BlockHeader const& _bi) { return _bi.seal<StakeSignature>(SignatureField); }
    static Nonce nonce(BlockHeader const& _bi) { return _bi.seal<Nonce>(NonceField); }
    h256 boundary(BlockHeader const& _bi) const;
    static BlockHeader& setSignature(BlockHeader& _bi, BLS::Signature _v) { _bi.setSeal(SignatureField, _v); return _bi; }
    static BlockHeader& setNonce(BlockHeader& _bi, Nonce _v) { _bi.setSeal(NonceField, _v); return _bi; }

	u256 calculateDifficulty(BlockHeader const& _bi, BlockHeader const& _parent) const;
	u256 childGasLimit(BlockHeader const& _bi, u256 const& _gasFloorTarget = Invalid256) const;

	void manuallySetWork(BlockHeader const& _work) { m_sealing = _work; }

	static void init();

    bool isMining() const { return m_generating; }
private:
	bool verifySeal(BlockHeader const& _bi) const;

	std::string m_sealer = "cpu";
	BlockHeader m_sealing;

    bool m_generating = false;
    std::thread sealThread;
	/// A mutex covering m_sealing
	Mutex m_submitLock;
};

}
}
