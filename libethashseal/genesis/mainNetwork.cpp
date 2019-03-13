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

static dev::h256 const c_genesisStateRootMainNetwork("bc228ba52e3db7b058ffc919eb65cdd4bf905fda1b39581a48fc431fcaa2fa2e");
static std::string const c_genesisInfoMainNetwork = std::string() +
R"E(
{
        "sealEngine": "Ethash",
        "params": {
                "accountStartNonce": "0x00",
                "homesteadForkBlock": "0x00",
                "daoHardforkBlock": "0x00",
                "byzantiumForkBlock": "0x00",
                "constantinopleForkBlock": "0xffffffffffffffffff",
                "networkID" : "0xF1",
                "maximumExtraDataSize": "0x20",
                "tieBreakingGas": false,
                "minGasLimit": "0x1388",
                "maxGasLimit": "7fffffffffffffff",
                "gasLimitBoundDivisor": "0x0400",
		"minimumDifficulty": "0x19d971e4fe8401e740000000",
		"initialSupply": "0x33b2e3c9fd0803ce8000000",
		"inflationFactorPerBlockFemtoPercent": "0x163459f9ed6f880",
                "difficultyBoundDivisor": "0x0800",
		"targetBlockInterval": "0x708",
                "durationLimit": "0x0d",
                "EIP150ForkBlock": "0x00",
                "EIP158ForkBlock": "0x00"
        },
        "genesis": {
                "nonce": "0x0000000000000042",
		"difficulty": "0x16b81d1a43b205ac3f40000000",
		"stakeModifier": "0x0000000000000000000b658e0935f5d7cf80cdb9f6fdd9aca928be07a9aeec16",
                "author": "0x0000000000000000000000000000000000000000",
		"timestamp": "5C884800",
                "parentHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
                "extraData":     "0x0000000000000000000b658e0935f5d7cf80cdb9f6fdd9aca928be07a9aeec16",
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
"f1117143371af98add6d269a6c15de38bee9b885": { "balance": "1000000000000000000000000000" }
        }
}
)E";
