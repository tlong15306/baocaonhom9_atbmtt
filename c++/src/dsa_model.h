#ifndef DSA_MODEL_H
#define DSA_MODEL_H

/*
 * DSAModel.h — Lớp xử lý thuật toán DSA (Digital Signature Algorithm)
 * Thư viện: GMP (GNU Multiple Precision) cho số học lớn
 *           OpenSSL (EVP API) cho SHA-256 và CSPRNG
 *
 * Nhóm 9 — An Toàn Bảo Mật Thông Tin, 2025-2026
 */

#include <string>
#include <gmp.h>

// ============================================================
// Struct: Dữ liệu trung gian kết quả xác minh
// Chứa tất cả các bước tính toán trung gian của quá trình
// xác minh chữ ký DSA để hiển thị trên giao diện
// ============================================================
struct VerifyDetails {
    std::string hash_hex;   // H(m) = SHA256(message) dạng hex
    std::string w_hex;      // w = s^(-1) mod q
    std::string u1_hex;     // u1 = H(m) * w mod q
    std::string u2_hex;     // u2 = r * w mod q
    std::string v_hex;      // v = (g^u1 * y^u2 mod p) mod q
    bool valid = false;     // Kết luận: v == r ?
};

// ============================================================
// Class: DSAModel
// Chứa toàn bộ logic thuật toán DSA
// Tham số hệ thống: (p, q, g)
//   - p: số nguyên tố lớn (1024 hoặc 2048 bit)
//   - q: số nguyên tố 256-bit, q | (p-1)
//   - g: phần tử sinh bậc q trong Z*_p
// Cặp khóa: (x, y)
//   - x: khóa bí mật (0 < x < q)
//   - y: khóa công khai, y = g^x mod p
// ============================================================
class DSAModel {
public:
    DSAModel();
    ~DSAModel(); // Gọi secureWipe() trước khi giải phóng

    // Ngăn copy (có tài nguyên GMP raw)
    DSAModel(const DSAModel&) = delete;
    DSAModel& operator=(const DSAModel&) = delete;

    // ----------------------------------------------------------
    // Nhóm 1: Sinh khóa
    // keyBits: 1024 hoặc 2048
    // Trả về true nếu thành công
    // ----------------------------------------------------------
    bool generateKeyPair(int keyBits);

    // ----------------------------------------------------------
    // Nhóm 2: Ký số
    // message: thông điệp văn bản cần ký
    // r_out, s_out: cặp chữ ký (r, s) dạng chuỗi HEX
    // ----------------------------------------------------------
    bool signMessage(const std::string& message,
                     std::string& r_out,
                     std::string& s_out);

    // ----------------------------------------------------------
    // Nhóm 3: Xác minh chữ ký
    // message: thông điệp gốc cần xác minh
    // r_hex, s_hex: cặp chữ ký dạng HEX
    // details: chi tiết các giá trị trung gian (w, u1, u2, v)
    // ----------------------------------------------------------
    bool verifySignature(const std::string& message,
                         const std::string& r_hex,
                         const std::string& s_hex,
                         VerifyDetails& details);

    // ----------------------------------------------------------
    // Nhóm 4: File I/O — Lưu/tải khóa và chữ ký
    // ----------------------------------------------------------
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

    // ----------------------------------------------------------
    // Nhóm 4: Dọn dẹp bộ nhớ an toàn
    // Xóa trắng vùng nhớ chứa khóa bí mật x
    // ----------------------------------------------------------
    void secureWipe();

    // ----------------------------------------------------------
    // Getters — Trả về chuỗi HEX để hiển thị trên UI
    // ----------------------------------------------------------
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
    // Tham số hệ thống (GMP big integers)
    mpz_t p_, q_, g_;      // System parameters
    mpz_t x_;              // Private key — BẢO MẬT TỐI ĐA
    mpz_t y_;              // Public key

    gmp_randstate_t randState_;  // Trạng thái CSPRNG (seeded từ OpenSSL RAND)
    bool randInit_ = false;

    std::string lastHash_hex_;   // Hash SHA-256 lần ký gần nhất
    int  keyBits_  = 0;
    bool keyReady_ = false;

    // ----------------------------------------------------------
    // Private helpers
    // ----------------------------------------------------------

    // Khởi tạo bộ sinh số ngẫu nhiên từ OpenSSL RAND_bytes
    void initRandom();

    // Chuyển mpz_t → chuỗi HEX (chữ hoa)
    std::string mpzToHex(const mpz_t n) const;

    // Chuyển chuỗi HEX → mpz_t
    void hexToMpz(mpz_t n, const std::string& hex) const;

    // Tính SHA-256(message) → chuỗi HEX 64 ký tự
    bool sha256Hex(const std::string& message, std::string& hashOut) const;

    // Tính SHA-256(message) → mpz_t (để dùng trong phép toán DSA)
    bool sha256ToMpz(const std::string& message, mpz_t result,
                     std::string& hexOut) const;
};

#endif // DSA_MODEL_H
