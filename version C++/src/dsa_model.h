#ifndef DSA_MODEL_H
#define DSA_MODEL_H

/*
 * DSAModel.h — Lớp xử lý thuật toán DSA (Digital Signature Algorithm)
 * Thư viện: GMP (GNU Multiple Precision) cho số học lớn
 * OpenSSL (EVP API) cho SHA-256 và CSPRNG
 *
 * Nhóm 9 — An Toàn Bảo Mật Thông Tin, 2025-2026
 */

#include <string>
#include <gmp.h>

// ============================================================
// Struct: Dữ liệu trung gian kết quả xác minh
// ============================================================
struct VerifyDetails {
    std::string hash_hex;   // H(m) = SHA256(message) dạng hex
    std::string w_hex;      // w = s^(-1) mod q
    std::string u1_hex;     // u1 = H(m) * w mod q
    std::string u2_hex;     // u2 = r * w mod q
    std::string v_hex;      // v = (g^u1 * y^u2 mod p) mod q
    bool valid = false;     // Kết luận: v == r ?
};

class DSAModel {
public:
    DSAModel();
    ~DSAModel(); // Gọi secureWipe() trước khi giải phóng

    // Ngăn copy (có tài nguyên GMP raw)
    DSAModel(const DSAModel&) = delete;
    DSAModel& operator=(const DSAModel&) = delete;

    bool generateKeyPair(int keyBits);

    bool signMessage(const std::string& message,
                     std::string& r_out,
                     std::string& s_out);

    bool verifySignature(const std::string& message,
                         const std::string& r_hex,
                         const std::string& s_hex,
                         const std::string& y_hex, // Thêm khóa công khai y
                         VerifyDetails& details);

    bool savePublicKey(const std::string& filepath) const;
    bool savePrivateKey(const std::string& filepath) const;
    bool saveSignature(const std::string& filepath,
                       const std::string& r_hex,
                       const std::string& s_hex) const;

    bool loadPublicKey(const std::string& filepath);
    bool loadPrivateKey(const std::string& filepath);
    bool loadSignature(const std::string& filepath,
                       std::string& r_out,
                       std::string& s_out);

    void secureWipe();

    std::string getP_hex()  const;
    std::string getQ_hex()  const;
    std::string getG_hex()  const;
    std::string getX_hex()  const;
    std::string getY_hex()  const;
    std::string getLastHash_hex() const;

    int  getPBitLength() const;
    int  getQBitLength() const;
    int  getKeyBits()    const { return keyBits_; }
    bool hasKeyPair()    const { return keyReady_; }

private:
    mpz_t p_, q_, g_;
    mpz_t x_;
    mpz_t y_;

    gmp_randstate_t randState_;
    bool randInit_ = false;

    std::string lastHash_hex_;
    int  keyBits_  = 0;
    bool keyReady_ = false;

    void initRandom();
    std::string mpzToHex(const mpz_t n) const;
    void hexToMpz(mpz_t n, const std::string& hex) const;
    bool sha256Hex(const std::string& message, std::string& hashOut) const;
    bool sha256ToMpz(const std::string& message, mpz_t result,
                     std::string& hexOut) const;
};

#endif // DSA_MODEL_H