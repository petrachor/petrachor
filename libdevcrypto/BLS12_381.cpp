#include "BLS12_381.h"

using namespace dev::BLS12_381;

using A8 = ArrayStruct8;
using A64 = ArrayStruct64;

extern "C" {

    void g1_get_generator(A8 result);
    void g2_get_generator(A8 result);

    void g1_add(A8 a, A8 b, A8 result);
    void g1_mul(A8 g, A64 p, A8 result);
    void g1_neg(A8 g, A8 result);

    void g2_add(A8 a, A8 b, A8 result);
    void g2_mul(A8 g, A64 p, A8 result);
    void g2_neg(A8 g, A8 result);

    void gt_mul(A64 a, A64 b, A64 result);
    void gt_inverse(A64 a, A64 result);

    void pairing(A8 g1, A8 g2, A64 gt);

    void hash_to_g1(A8 seed, A8 data, A8 result);
    void hash_to_g2(A8 seed, A8 data, A8 result);
}

namespace dev {
    namespace BLS12_381 {

        ArrayStruct8 G1::toAS() { return ArrayStruct8{data(), size}; }
        ArrayStruct8 G2::toAS() { return ArrayStruct8{data(), size}; }
        ArrayStruct64 GT::toAS() { return ArrayStruct64{(long unsigned int*) data(), size}; }
        ArrayStruct64 Scalar::toAS() { return ArrayStruct64{(long unsigned int*) data(), size}; }

        ArrayStruct8 toAS(bytesConstRef b) { return ArrayStruct8{(unsigned char*) b.data(), b.size()}; }

        G1 G1::getGenerator() { G1 r; g1_get_generator(r.toAS()); return r; }
        G2 G2::getGenerator() { G2 r; g2_get_generator(r.toAS()); printf("Received generator %s", r.hex().c_str()); return r; }

        G1 G1::add(G1 s) { G1 r; g1_add(toAS(), s.toAS(), r.toAS()); return r; }
        G2 G2::add(G2 s) { G2 r; g2_add(toAS(), s.toAS(), r.toAS()); return r; }

        G1 G1::mul(Scalar s) { G1 r; g1_mul(toAS(), s.toAS(), r.toAS()); return r; }
        G2 G2::mul(Scalar s) { G2 r;
                               g2_mul(toAS(), s.toAS(), r.toAS());
                         printf("Mul g = %s scalar = %s result = %s", hex().c_str(), s.makeInsecure().hex().c_str(), r.hex().c_str());
                                                                     return r; }

        G1 G1::neg() { G1 r; g1_neg(toAS(), r.toAS()); return r; }
        G2 G2::neg() { G2 r; g2_neg(toAS(), r.toAS()); return r; }

        G1 G1::mapToElement(bytesConstRef seed, bytesConstRef data) { G1 g; hash_to_g1(::toAS(seed), ::toAS(data), g.toAS()); return g; }
        G2 G2::mapToElement(bytesConstRef seed, bytesConstRef data) { G2 g; hash_to_g2(::toAS(seed), ::toAS(data), g.toAS()); return g; }

        GT GT::fromPairing(G1 g1, G2 g2) { GT r; pairing(g1.toAS(), g2.toAS(), r.toAS()); return r; }

        GT GT::mul(GT other) { GT r; gt_mul(toAS(), other.toAS(), r.toAS()); return r; }
        GT GT::inv() { GT r; gt_inverse(toAS(), r.toAS()); return r; }

        G2 BonehLynnShacham::generatePublicKey(const Scalar& secret) { return G2::getGenerator().mul(secret); }

        G1 BonehLynnShacham::sign(G1 element, const Scalar &secret) { return element.mul(secret); }

        bool BonehLynnShacham::verify(G2 publicKey, G1 hashedMessage, G1 signedHashedMessage) {
            GT a = GT::fromPairing(signedHashedMessage, G2::getGenerator());
            GT b = GT::fromPairing(hashedMessage, publicKey);
            return a == b;
        }

        std::pair<bool, bytes> bls12_381_pairing(bytesConstRef _in) { return { true, GT::fromPairing(G1(_in), G2(_in.cropped(G1::size))).asBytes() }; }

        std::pair<bool, bytes> bls12_381_G1_add(bytesConstRef _in) { return { true, G1(_in).add(G1(_in.cropped(G1::size))).asBytes() }; }
        std::pair<bool, bytes> bls12_381_G1_mul(bytesConstRef _in) { return { true, G1(_in).mul(Scalar(_in.cropped(G1::size))).asBytes() }; }
        std::pair<bool, bytes> bls12_381_G1_neg(bytesConstRef _in) { return { true, G1(_in).neg().asBytes() }; }

        std::pair<bool, bytes> bls12_381_G2_add(bytesConstRef _in) { return { true, G2(_in).add(G2(_in.cropped(G2::size))).asBytes() }; }
        std::pair<bool, bytes> bls12_381_G2_mul(bytesConstRef _in) { return { true, G2(_in).mul(Scalar(_in.cropped(G2::size))).asBytes() }; }
        std::pair<bool, bytes> bls12_381_G2_neg(bytesConstRef _in) { return { true, G2(_in).neg().asBytes() }; }

        std::pair<bool, bytes> bls12_381_GT_mul(bytesConstRef _in) { return { true, GT(_in).mul(GT(_in.cropped(GT::size))).asBytes() }; }
        std::pair<bool, bytes> bls12_381_GT_inv(bytesConstRef _in) { return { true, GT(_in).inv().asBytes() }; }

    }
}
