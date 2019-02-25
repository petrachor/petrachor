#include "BLS12_381.h"

#include <exception>

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

        G1 G1::getOne() { G1 r; g1_get_one(r.toAS()); return r; }
        G2 G2::getOne() { G2 r; g2_get_one(r.toAS()); return r; }
        GT GT::getOne() { GT r; gt_get_one(r.toAS()); return r; }
        G1 G1::getZero() { G1 r; g1_get_zero(r.toAS()); return r; }
        G2 G2::getZero() { G2 r; g2_get_zero(r.toAS()); return r; }
        GT GT::getZero() { GT r; gt_get_zero(r.toAS()); return r; }

        G1 G1::add(G1 const& s) const { G1 r; g1_add(G1(*this).toAS(), G1(s).toAS(), r.toAS()); return r; }
        G2 G2::add(G2 const& s) const { G2 r; g2_add(G2(*this).toAS(), G2(s).toAS(), r.toAS()); return r; }

        G1 G1::mul(Scalar const& s) const { G1 r; g1_mul(G1(*this).toAS(), Scalar(s).toAS(), r.toAS()); return r; }
        G2 G2::mul(Scalar const& s) const { G2 r; g2_mul(G2(*this).toAS(), Scalar(s).toAS(), r.toAS()); return r; }

        G1 G1::neg() const { G1 r; g1_neg(G1(*this).toAS(), r.toAS()); return r; }
        G2 G2::neg() const { G2 r; g2_neg(G2(*this).toAS(), r.toAS()); return r; }

        G1 operator * (Scalar const& s, G1 const& a) { return a.mul(s); }
        G2 operator * (Scalar const& s, G2 const& a) { return a.mul(s); }
        G1 operator * (G1 const& a, Scalar const& s) { return s * a; }
        G2 operator * (G2 const& a, Scalar const& s) { return s * a; }

        G1 operator + (G1 const& a, G1 const& b) { return a.add(b); }
        G2 operator + (G2 const& a, G2 const& b) { return a.add(b); }

        G1 G1::mapToElement(bytesConstRef seed, bytesConstRef data) {
            assert(seed.size() == sizeof(u64));
            assert(data.size() == sizeof(u64));
            G1 g; hash_to_g1(::toAS(seed), ::toAS(data), g.toAS()); return g;
        }
        G2 G2::mapToElement(bytesConstRef seed, bytesConstRef data) {
            assert(seed.size() == sizeof(u64));
            assert(data.size() == sizeof(u64));
            G2 g; hash_to_g2(::toAS(seed), ::toAS(data), g.toAS()); return g;
        }

        GT GT::fromPairing(G1 const& g1, G2 const& g2) { GT r; pairing(G1(g1).toAS(), G2(g2).toAS(), r.toAS()); return r; }
        GT GT::fromMultiPairing(G1G2s const& gs) {
            GT result = GT::getOne();
            for (auto const& pair: gs) { // TODO: Optimize by avoiding multiple final exps (later)
                result = result.mul(GT::fromPairing(pair.first, pair.second));
            }
            return result;
        }

        GT GT::mul(GT const& other) const { GT r; gt_mul(GT(*this).toAS(), GT(other).toAS(), r.toAS()); return r; }
        GT GT::inv() const { GT r; gt_inverse(GT(*this).toAS(), r.toAS()); return r; }

        Scalar Scalar::random() { Scalar r; *(SecureFixedHash*) &r = SecureFixedHash::random(); return r; }

        G2 BonehLynnShacham::generatePublicKey(Scalar const& secret) { return G2::publicFromPrivateKey(secret); }

        G1 BonehLynnShacham::sign(G1 const& element, Scalar const& secret) { return secret * element; }

        bool BonehLynnShacham::verify(G2 publicKey, G1 hashedMessage, G1 signedHashedMessage) {
            GT a = GT::fromPairing(signedHashedMessage, G2::getOne());
            GT b = GT::fromPairing(hashedMessage, publicKey);
            return a == b;
        }

    }
}
