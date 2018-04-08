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
#include "../GenesisInfo.h"

static dev::h256 const c_genesisStateRootMainNetwork("395a37cc097c03c6d10aab8b0ea2bc121a9806ce741ddf061943ee8d5d97f092");
static std::string const c_genesisInfoMainNetwork = std::string() +
R"E(
{
	"sealEngine": "Ethash",
	"params": {
		"accountStartNonce": "0x00",
                "homesteadForkBlock": "0x00",
                "daoHardforkBlock": "0x00",
                "EIP150ForkBlock": "0x00",
                "EIP158ForkBlock": "0x00",
                "byzantiumForkBlock": "0x00",
                "constantinopleForkBlock": "0xffffffffffffffffff",
		"networkID" : "0x01",
		"chainID": "0x01",
		"maximumExtraDataSize": "0x20",
		"tieBreakingGas": false,
		"minGasLimit": "0x1388",
		"maxGasLimit": "7fffffffffffffff",
		"gasLimitBoundDivisor": "0x0400",
		"minimumDifficulty": "0x020000",
		"difficultyBoundDivisor": "0x0800",
		"durationLimit": "0x0d",
		"blockReward": "0x4563918244F40000"
	},
	"genesis": {
		"nonce": "0x0000000000000042",
    "difficulty": "0x40000000000000000",
                "stakeModifier": "0x11bbe8db4e347b4e8c937c1c8370e4b5ed33adb3db69cbdb7a38e1e50b1b82fa",
		"author": "0x0000000000000000000000000000000000000000",
		"timestamp": "0x00",
		"parentHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
                "extraData":     "0x11bbe8db4e347b4e8c937c1c8370e4b5ed33adb3db69cbdb7a38e1e50b1b82fa",
		"gasLimit": "0x781388"
	},
	"accounts": {
		"0000000000000000000000000000000000000001": { "precompiled": { "name": "ecrecover", "linear": { "base": 3000, "word": 0 } } },
		"0000000000000000000000000000000000000002": { "precompiled": { "name": "sha256", "linear": { "base": 60, "word": 12 } } },
		"0000000000000000000000000000000000000003": { "precompiled": { "name": "ripemd160", "linear": { "base": 600, "word": 120 } } },
		"0000000000000000000000000000000000000004": { "precompiled": { "name": "identity", "linear": { "base": 15, "word": 3 } } },
		"0000000000000000000000000000000000000005": { "precompiled": { "name": "modexp", "startingBlock": "0xffffffffffffffffff" } },
		"0000000000000000000000000000000000000006": { "precompiled": { "name": "alt_bn128_G1_add", "startingBlock": "0xffffffffffffffffff", "linear": { "base": 500, "word": 0 } } },
		"0000000000000000000000000000000000000007": { "precompiled": { "name": "alt_bn128_G1_mul", "startingBlock": "0xffffffffffffffffff", "linear": { "base": 2000, "word": 0 } } },
                "0000000000000000000000000000000000000008": { "precompiled": { "name": "alt_bn128_pairing_product", "startingBlock": "0xffffffffffffffffff" } },
"0000000000000000000000010000000000000000": { "precompiled": { "name": "bls12_381_g1", "startingBlock": "0xffffffffffffffffff", "linear": { "base": 500, "word": 0 } } },
"0000000000000000000000010000000000000000": { "precompiled": { "name": "bls12_381_g2", "startingBlock": "0xffffffffffffffffff", "linear": { "base": 500, "word": 0 } } },
"0000000000000000000000010000000000000000": { "precompiled": { "name": "bls12_381_gt", "startingBlock": "0xffffffffffffffffff", "linear": { "base": 500, "word": 0 } } },
"0000000000000000000000010000000000000000": { "precompiled": { "name": "bls12_381_pairing", "startingBlock": "0xffffffffffffffffff", "linear": { "base": 500, "word": 0 } } },
"0000000000000000000000010000000000000000": { "precompiled": { "name": "bls12_381_multipairing", "startingBlock": "0xffffffffffffffffff", "linear": { "base": 500, "word": 0 } } },
"003ff0f596f8508b96e7c7f3eb6f8196d5f2309d": { "balance": "1000000000000000000" },
"00a1fa895297ad0412e867ca57f4dcfd5682f435": { "balance": "1000000000000000000" },
"00ca0701dd80a8e9cd2e54cbcc74e97451683408": { "balance": "1000000000000000000" },
"0087f05a319eea6110db401ecd2ee41e7f0b92c4": { "balance": "1000000000000000000" },
"0035d2239538c482d99ae144097ab3fdc8861902": { "balance": "1000000000000000000" }
        }
}
)E";
