#ifndef BLS12_381_H
#define BLS12_381_H

#include <libdevcore/Address.h>
#include <libdevcore/Common.h>
#include <libdevcore/Exceptions.h>
#include <libdevcore/FixedHash.h>

namespace dev {
    using H48 = FixedHash<48>;
    using H96 = FixedHash<96>;
    using H192 = FixedHash<192>;
    using H12_48 = FixedHash<12*48>;
}

namespace dev {

    namespace BLS12_381 {

        struct ArrayStruct8 {
            unsigned char* d;
            size_t length;
        };

        struct ArrayStruct64 {
            unsigned long* d;
            size_t length;
        };

        class Scalar: public SecureFixedHash<32> {
        public:
            ArrayStruct64 toAS();

            static Scalar random();
            Scalar() : SecureFixedHash() {}
            Scalar(SecureFixedHash h) : SecureFixedHash(h) {}
            Scalar(bytes d) : SecureFixedHash(d) { }
            Scalar(FixedHash<32> d) : SecureFixedHash(d.asBytes()) { }
            Scalar(bytesSec d) : SecureFixedHash(d) { }
            Scalar(bytesRef d) : SecureFixedHash(d) { }
            Scalar(bytesConstRef d) : SecureFixedHash(d) { }

            Scalar(std::string hex) : SecureFixedHash<32>(hex) { }
        };

        class G1 : public H48 {
        public:
            static G1 getOne();
            static G1 getZero();
            static G1 mapToElement(bytesConstRef seed, bytesConstRef data);
            static G1 publicFromPrivateKey(Scalar privateKey) { return getOne().mul(privateKey); }

            G1() : H48() {}
            G1(bytesConstRef d) : H48(d) { }
            G1 mul(Scalar s);
            G1 add(G1 other);
            G1 neg();

            ArrayStruct8 toAS();
        };

        class G2 : public H96 {
        public:
            static G2 getOne();
            static G2 getZero();
            static G2 mapToElement(bytesConstRef seed, bytesConstRef data);
            static G2 publicFromPrivateKey(Scalar privateKey) { return getOne().mul(privateKey); }

            G2() : H96() {}
            G2(const G2& g) : H96(g) {}
            G2(dev::FixedHash<96> h) : H96(h) { }
            G2(bytesConstRef d) : H96(d) { }
            G2(bytes d) : H96(d) { }

            G2(dev::FixedHash<32> h) { *this = G2::publicFromPrivateKey(Scalar(h)); }
//            G2(std::string s) : H96(bytesConstRef(s)) { }
            G2 mul(Scalar s);
            G2 add(G2 other);
            G2 neg();

            ArrayStruct8 toAS();
        };

        typedef std::pair<G1, G2> G1G2;
        typedef std::vector<G1G2> G1G2s;

        class GT : public H12_48 {
        public:
            static GT getOne();
            static GT getZero();
            static GT fromPairing(G1 g1, G2 g2);
            static GT fromMultiPairing(G1G2s gs);

            GT() : H12_48() {}
            GT(bytesConstRef d) : H12_48(d) { }
            GT mul(GT other);
            GT inv();

            ArrayStruct64 toAS();
        };

        class BonehLynnShacham {
        public:
            static G2 generatePublicKey(const Scalar& secret);
            static G1 sign(G1 element, const Scalar& secret);
            static bool verify(G2 publicKey, G1 element, G1 signedElement);
        };

    }

}

namespace std
{
    /// Forward std::hash<dev::FixedHash> to dev::FixedHash::hash.
#define forwardHash(T) template<> struct hash<T>: T::hash {};

    template<> struct hash<dev::H48>: dev::H48::hash {};
    template<> struct hash<dev::H96>: dev::H96::hash {};
    template<> struct hash<dev::H192>: dev::H192::hash {};
    template<> struct hash<dev::H12_48>: dev::H12_48::hash {};
    template<> struct hash<dev::BLS12_381::G2>: dev::BLS12_381::G2::hash {};
}


#endif // BLS12_381_H
