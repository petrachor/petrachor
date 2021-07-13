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
/** @file ChainOperationParams.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2015
 */

#include "ChainOperationParams.h"
#include <libdevcore/Log.h>
#include <libdevcore/CommonData.h>
using namespace std;
using namespace dev;
using namespace eth;

PrecompiledContract::PrecompiledContract(
	unsigned _base,
	unsigned _word,
	PrecompiledExecutor const& _exec,
	u256 const& _startingBlock
):
	PrecompiledContract([=](bytesConstRef _in) -> bigint
	{
		bigint s = _in.size();
		bigint b = _base;
		bigint w = _word;
		return b + (s + 31) / 32 * w;
	}, _exec, _startingBlock)
{}

ChainOperationParams::ChainOperationParams():
	minGasLimit(0x1388),
	maxGasLimit("0x7fffffffffffffff"),
	gasLimitBoundDivisor(0x0400),
	networkID(0x0),
	minimumDifficulty(0x020000),
	difficultyBoundDivisor(0x0800),
	durationLimit(0x0d),
	registrar("0x5e70c0bbcd5636e0f9f9316e9f8633feb64d4050")
{
}

EVMSchedule const& ChainOperationParams::scheduleForBlockNumber(u256 const& _blockNumber) const
{
    if (_blockNumber >= istanbulForkBlock)
        return IstanbulSchedule;
    if (_blockNumber >= constantinopleForkBlock)
        return ConstantinopleSchedule;
    else return ByzantiumSchedule;

}
