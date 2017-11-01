#ifndef BLS12_381_H
#define BLS12_381_H

#include <libdevcrypto/Common.h>

namespace dev {

    namespace BLS12_381 {

        using H48 = FixedHash<48>;
        using H96 = FixedHash<96>;
        using H192 = FixedHash<192>;
        using H12_48 = FixedHash<12*48>;

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

            Scalar() : SecureFixedHash() {}
            Scalar(bytesConstRef d) : SecureFixedHash(d) { }

            Scalar(std::string hex) : SecureFixedHash<32>(hex) { }
        };

        class G1 : public H96 {
        public:
            static G1 getGenerator();
            static G1 mapToElement(bytesConstRef seed, bytesConstRef data);

            G1() : H96() {}
            G1(bytesConstRef d) : H96(d) { }
            G1 mul(Scalar s);
            G1 add(G1 other);
            G1 neg();

            ArrayStruct8 toAS();
        };

        class G2 : public H192 {
        public:
            static G2 getGenerator();
            static G2 mapToElement(bytesConstRef seed, bytesConstRef data);

            G2() : H192() {}
            G2(bytesConstRef d) : H192(d) { }
            G2 mul(Scalar s);
            G2 add(G2 other);
            G2 neg();

            ArrayStruct8 toAS();
        };

        class GT : public H12_48 {
        public:
            static GT fromPairing(G1 g1, G2 g2);

            GT() : H12_48() {}
            GT(bytesConstRef d) : H12_48(d) { }
            GT mul(GT other);
            GT inv();

            ArrayStruct64 toAS();
        };

        class BonehLynnShacham {
        public:
            G2 generatePublicKey(const Scalar& secret);
            G1 sign(G1 element, const Scalar& secret);
            bool verify(G2 publicKey, G1 element, G1 signedElement);
        };

        std::pair<bool, bytes> bls12_381_pairing(bytesConstRef _in);

        std::pair<bool, bytes> bls12_381_G1_add(bytesConstRef _in);
        std::pair<bool, bytes> bls12_381_G1_mul(bytesConstRef _in);
        std::pair<bool, bytes> bls12_381_G1_neg(bytesConstRef _in);

        std::pair<bool, bytes> bls12_381_G2_add(bytesConstRef _in);
        std::pair<bool, bytes> bls12_381_G2_mul(bytesConstRef _in);
        std::pair<bool, bytes> bls12_381_G2_neg(bytesConstRef _in);

        std::pair<bool, bytes> bls12_381_GT_mul(bytesConstRef _in);
        std::pair<bool, bytes> bls12_381_GT_inv(bytesConstRef _in);

    }

}

#endif // BLS12_381_H
