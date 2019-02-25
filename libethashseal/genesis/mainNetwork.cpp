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

static dev::h256 const c_genesisStateRootMainNetwork("fc3accc27dfd8be52f886ff67c75e53431ac83358b295f143ce2a89279bf6de8");
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
		"networkID" : "0xF1",
		"chainID": "0xF1",
		"maximumExtraDataSize": "0x20",
		"tieBreakingGas": false,
		"minGasLimit": "0x1388",
		"maxGasLimit": "7fffffffffffffff",
		"gasLimitBoundDivisor": "0x0400",
                "minimumDifficulty": "0x0200",
		"difficultyBoundDivisor": "0x0800",
                "targetBlockInterval": "0x0A",
		"durationLimit": "0x0d",
		"blockReward": "0x4563918244F40000"
	},
	"genesis": {
		"nonce": "0x0000000000000042",
    "difficulty": "0x989680",
                "stakeModifier": "0x11bbe8db4e347b4e8c937c1c8370e4b5ed33adb3db69cbdb7a38e1e50b1b82fa",
		"author": "0x0000000000000000000000000000000000000000",
                "timestamp": "5c6c85ba",
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
"009fe73456cf0175fc0dfb4e2d5c932a60d1d977": { "balance": "1000000" },
"00e7dd85e346c2f9eedca12179d90316a77e82ec": { "balance": "1000000" },
"00501c1aae2d8d4e5ec636cc1fd7e54503304100": { "balance": "1000000" },
"00e1e1ae4a135b7a96fce02610324423373e3e7f": { "balance": "1000000" }
        }
}
)E";
