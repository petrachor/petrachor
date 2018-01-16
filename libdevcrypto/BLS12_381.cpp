#include "BLS12_381.h"

using namespace dev::BLS12_381;

using A8 = ArrayStruct8;
using A64 = ArrayStruct64;

extern "C" {

    void g1_get_one(A8 result);
    void g2_get_one(A8 result);
    void gt_get_one(A64 result);

    void g1_get_zero(A8 result);
    void g2_get_zero(A8 result);
    void gt_get_zero(A64 result);

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

//    void unsafe(std::function<void()> f) { try { f(); } catch (...) { } }
    void unsafe(std::function<void()> f) { f(); }

        ArrayStruct8 G1::toAS() { return ArrayStruct8{data(), size}; }
        ArrayStruct8 G2::toAS() { return ArrayStruct8{data(), size}; }
        ArrayStruct64 GT::toAS() { return ArrayStruct64{(long unsigned int*) data(), size/sizeof(u64)}; }
        ArrayStruct64 Scalar::toAS() { return ArrayStruct64{(long unsigned int*) data(), size/sizeof(u64)}; }

        ArrayStruct8 toAS(bytesConstRef b) { return ArrayStruct8{(unsigned char*) b.data(), b.size()}; }

        G1 G1::getOne() { G1 r; unsafe([&](){ g1_get_one(r.toAS()); }); return r; }
        G2 G2::getOne() { G2 r; unsafe([&](){ g2_get_one(r.toAS()); }); return r; }
        GT GT::getOne() { GT r; unsafe([&](){ gt_get_one(r.toAS()); }); return r; }
        G1 G1::getZero() { G1 r; unsafe([&](){ g1_get_zero(r.toAS()); }); return r; }
        G2 G2::getZero() { G2 r; unsafe([&](){ g2_get_zero(r.toAS()); }); return r; }
        GT GT::getZero() { GT r; unsafe([&](){ gt_get_zero(r.toAS()); }); return r; }

        G1 G1::add(G1 s) { G1 r; unsafe([&](){ g1_add(toAS(), s.toAS(), r.toAS()); }); return r; }
        G2 G2::add(G2 s) { G2 r; unsafe([&](){ g2_add(toAS(), s.toAS(), r.toAS()); }); return r; }

        G1 G1::mul(Scalar s) { G1 r; unsafe([&](){ g1_mul(toAS(), s.toAS(), r.toAS()); }); return r; }
        G2 G2::mul(Scalar s) { G2 r; unsafe([&](){ g2_mul(toAS(), s.toAS(), r.toAS()); }); return r; }

        G1 G1::neg() { G1 r; unsafe([&](){ g1_neg(toAS(), r.toAS()); }); return r; }
        G2 G2::neg() { G2 r; unsafe([&](){ g2_neg(toAS(), r.toAS()); }); return r; }

        G1 G1::mapToElement(bytesConstRef seed, bytesConstRef data) {
            G1 g; unsafe([&](){ hash_to_g1(::toAS(seed), ::toAS(data), g.toAS()); }); return g;
        }
        G2 G2::mapToElement(bytesConstRef seed, bytesConstRef data) {
            G2 g; unsafe([&](){ hash_to_g2(::toAS(seed), ::toAS(data), g.toAS()); }); return g;
        }

        GT GT::fromPairing(G1 g1, G2 g2) { GT r; unsafe([&](){ pairing(g1.toAS(), g2.toAS(), r.toAS()); }); return r; }
        GT GT::fromMultiPairing(G1G2s gs) {
            GT result = GT::getOne();
            for (auto const& pair: gs) { // TODO: Optimize by avoiding multiple final exps (later)
                result = result.mul(GT::fromPairing(pair.first, pair.second));
            }
            return result;
        }

        GT GT::mul(GT other) { GT r; unsafe([&](){ gt_mul(toAS(), other.toAS(), r.toAS()); }); return r; }
        GT GT::inv() { GT r; unsafe([&](){ gt_inverse(toAS(), r.toAS()); }); return r; }

        Scalar Scalar::random() { Scalar r; *(SecureFixedHash*) &r = SecureFixedHash::random(); return r; }

        G2 BonehLynnShacham::generatePublicKey(const Scalar& secret) { return G2::publicFromPrivateKey(secret); }

        G1 BonehLynnShacham::sign(G1 element, const Scalar &secret) { return element.mul(secret); }

        bool BonehLynnShacham::verify(G2 publicKey, G1 hashedMessage, G1 signedHashedMessage) {
            GT a = GT::fromPairing(signedHashedMessage, G2::getOne());
            GT b = GT::fromPairing(hashedMessage, publicKey);
            return a == b;
        }

    }
}
