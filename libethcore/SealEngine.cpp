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

#include "SealEngine.h"
#include "Transaction.h"

using namespace std;
using namespace dev;
using namespace eth;

SealEngineRegistrar* SealEngineRegistrar::s_this = nullptr;

void NoProof::init()
{
	ETH_REGISTER_SEAL_ENGINE(NoProof);
}

void SealEngineFace::verify(Strictness _s, BlockHeader const& _bi, BalanceRetriever , BlockHeader const& _parent, bytesConstRef _block) const
{
	_bi.verify(_s, _parent, _block);
}

void SealEngineFace::populateFromParent(BlockHeader& _bi, BlockHeader const& _parent)
{
	_bi.populateFromParent(_parent);
}

void SealEngineFace::verifyTransaction(ImportRequirements::value _ir, TransactionBase const& _t, BlockHeader const& _header, u256 const&) const
{
    if ((_ir & ImportRequirements::TransactionSignatures) && _header.number() < chainParams().EIP158ForkBlock)
		BOOST_THROW_EXCEPTION(InvalidSignature());

	if ((_ir & ImportRequirements::TransactionSignatures) && _header.number() < chainParams().constantinopleForkBlock && _t.hasZeroSignature())
		BOOST_THROW_EXCEPTION(InvalidSignature());

	if ((_ir & ImportRequirements::TransactionBasic) &&
		_header.number() >= chainParams().constantinopleForkBlock &&
		_t.hasZeroSignature() &&
		(_t.value() != 0 || _t.gasPrice() != 0 || _t.nonce() != 0))
			BOOST_THROW_EXCEPTION(InvalidZeroSignatureTransaction() << errinfo_got((bigint)_t.gasPrice()) << errinfo_got((bigint)_t.value()) << errinfo_got((bigint)_t.nonce()));

	if (_header.number() >= chainParams().homesteadForkBlock && (_ir & ImportRequirements::TransactionSignatures) && _t.hasSignature())
		_t.checkLowS();
}

SealEngineFace* SealEngineRegistrar::create(ChainOperationParams const& _params)
{
	SealEngineFace* ret = create(_params.sealEngineName);
	assert(ret && "Seal engine not found.");
	if (ret)
		ret->setChainParams(_params);
	return ret;
}

EVMSchedule const& SealEngineBase::evmSchedule(u256 const& _blockNumber) const
{
	return chainParams().scheduleForBlockNumber(_blockNumber);
}

u256 SealEngineBase::blockReward(unsigned long long const& _blockNumber) const
{
    u256 intReward = 0;
    if (_blockNumber > 0) {
        bigfloat initialSupply = chainParams().initialSupply.convert_to<bigfloat>();
        bigfloat f = chainParams().inflationFactorPerBlockFemtoPercent.convert_to<bigfloat>()/bigfloat("1E+17");
        bigfloat supply = initialSupply * pow(f, _blockNumber - 1);
        bigfloat reward = (supply * f) - supply;

        intReward = reward.convert_to<u256>();
//        std::cout << "Ifpbfp " << chainParams().inflationFactorPerBlockFemtoPercent << " BlockReward digits: " << std::numeric_limits<bigfloat>::digits << " " << std::numeric_limits<bigfloat>::max_digits10 << std::setprecision(std::numeric_limits<bigfloat>::max_digits10)
//             << " initialSupply = " << initialSupply << " ifpb = " << f << " supply at block " << (_blockNumber - 1) << " = " << supply << " supply * f = " << (supply * f) << " reward = " << reward << std::endl << std::flush;
    }
    return intReward;
}
