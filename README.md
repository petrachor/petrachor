# Petrachor C++ client

This repository contains the [Petrachor](https://petrachor.com) C++ Client.


## Getting Started

**[Installation Instructions](https://https://github.com/petrachor/petrachor/wiki/Installation-Instructions)**.


## About Petrachor

Petrachor uses the Ariel PoS protocol, which is our custom implementation of the original iChing Proof-of-Stake consensus, a secure and eco-friendly Satoshi-style protocol. iChing is a natural mimic of Bitcoin consensus; as a result, scalability solutions for Bitcoin can be immediately used in Petrachor. By applying this blockchain protocol to a fork of the Ethereum source code, we deliver a powerful new dApp platform combining the energy-efficiency of PoS with the power and versatility of Ethereum.

**Proof of Stake**

Keeping in the spirit of naming consensi after ghosts, Ariel incarnates an academic protocol that was proven to satisfy important formal chain properties including common prefix, chain quality, chain growth, and chain soundness.

**BLS Signature Scheme**

The Boneh-Lynn-Shacham unique signature scheme is used for the core random beacon of Ariel. By also securing accounts with BLS, we pave the road towards interesting future extensions, such as multisignatures, threshold signatures and aggregate signatures.

**BLS12-381 Elliptic Curve**

The elliptic curve used by our signature scheme is BLS12-381, a Barreto-Lynn-Scott curve. This curve was introduced in Sapling, the latest upgrade to the ZCash protocol, with the aim to improve efficiency and security while reducing memory intensivity.


## References
* [Short signatures from the Weil pairing](https://www.iacr.org/archive/asiacrypt2001/22480516.pdf)
* [BLS Multi-Signatures With Public-Key Aggregation](https://crypto.stanford.edu/~dabo/pubs/papers/BLSmultisig.html)
* [A Scalable Proof-of-Stake Blockchain in the Open Setting](https://eprint.iacr.org/2017/656.pdf)


## Documentation

[Wiki](https://github.com/petrachor/petrachor/wiki)



## License

[![License](https://img.shields.io/github/license/ethereum/cpp-ethereum.svg)](LICENSE)

All contributions are made under the [GNU General Public License v3](https://www.gnu.org/licenses/gpl-3.0.en.html). See [LICENSE](LICENSE).
