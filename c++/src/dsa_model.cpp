/*
 * dsa_model.cpp — Triển khai thuật toán DSA tự viết bằng GMP + OpenSSL
 *
 * Nhóm 9 — An Toàn Bảo Mật Thông Tin, 2025-2026
 *
 * Thư viện sử dụng:
 *   GMP  (libgmp)    — Số học với số nguyên lớn (mpz_t)
 *   OpenSSL (libcrypto) — SHA-256 (EVP API) và CSPRNG (RAND_bytes)
 */

#include "dsa_model.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <ctime>

// ============================================================
// Constructor / Destructor
// ============================================================

DSAModel::DSAModel() : randInit_(false), keyBits_(0), keyReady_(false) {
    // Khởi tạo tất cả biến GMP
    mpz_inits(p_, q_, g_, x_, y_, nullptr);
    initRandom();
}

DSAModel::~DSAModel() {
    secureWipe();
    mpz_clears(p_, q_, g_, x_, y_, nullptr);
    if (randInit_) {
        gmp_randclear(randState_);
    }
}

// ============================================================
// Khởi tạo bộ sinh số ngẫu nhiên an toàn
// Seed từ OpenSSL RAND_bytes (CSPRNG)
// ============================================================
void DSAModel::initRandom() {
    if (randInit_) {
        gmp_randclear(randState_);
    }

    // Dùng Mersenne Twister của GMP (gmp_randinit_mt)
    gmp_randinit_mt(randState_);

    // Lấy seed 32 byte từ OpenSSL (CSPRNG)
    unsigned char seedBytes[32];
    if (RAND_bytes(seedBytes, sizeof(seedBytes)) != 1) {
        // Fallback: dùng thời gian
        unsigned long t = (unsigned long)time(nullptr);
        gmp_randseed_ui(randState_, t);
    } else {
        mpz_t mpzSeed;
        mpz_init(mpzSeed);
        mpz_import(mpzSeed, sizeof(seedBytes), 1, 1, 0, 0, seedBytes);
        gmp_randseed(randState_, mpzSeed);
        mpz_clear(mpzSeed);
        // Xóa seed khỏi bộ nhớ
        OPENSSL_cleanse(seedBytes, sizeof(seedBytes));
    }

    randInit_ = true;
}

// ============================================================
// NHÓM 1: Sinh cặp khóa DSA
//
// keyBits: kích thước p (1024 hoặc 2048 bit)
//
// Quy trình:
// 1. Sinh q: số nguyên tố 256-bit
// 2. Sinh p: số nguyên tố keyBits-bit, q | (p-1)
//    p = m*q + 1, tìm m sao cho p nguyên tố
// 3. Tính g = h^((p-1)/q) mod p, h ngẫu nhiên trong Z*_p
//    Lặp đến khi g != 1
// 4. Sinh x: 0 < x < q (khóa bí mật)
// 5. Tính y = g^x mod p (khóa công khai)
// ============================================================
bool DSAModel::generateKeyPair(int keyBits) {
    keyBits_  = keyBits;
    keyReady_ = false;

    // Tái khởi tạo bộ sinh số ngẫu nhiên
    initRandom();

    mpz_t temp, m, h, exp;
    mpz_inits(temp, m, h, exp, nullptr);

    // --------------------------------------------------------
    // Bước 1: Sinh q — số nguyên tố 256-bit
    // Phương pháp: sinh số ngẫu nhiên 256-bit, đặt bit cao nhất
    // và bit thấp nhất = 1 (bắt buộc lẻ), kiểm tra nguyên tố
    // --------------------------------------------------------
    do {
        mpz_urandomb(q_, randState_, 256);
        mpz_setbit(q_, 255); // Đảm bảo q có đúng 256 bit
        mpz_setbit(q_, 0);   // Đảm bảo q lẻ (điều kiện cần để nguyên tố)
    } while (mpz_probab_prime_p(q_, 50) == 0);
    // mpz_probab_prime_p(n, reps): 50 lần kiểm tra Miller-Rabin

    // --------------------------------------------------------
    // Bước 2: Sinh p — số nguyên tố keyBits-bit, q | (p-1)
    // Chiến lược: p = m*q + 1
    //   m có (keyBits - bitlen(q)) bit
    //   Tìm m sao cho p nguyên tố
    //
    // Xác suất thành công ~1/ln(2^keyBits) = ~1/710 (với 1024 bit)
    // Trung bình cần ~1000 thử với 1024-bit
    // --------------------------------------------------------
    int qbits  = (int)mpz_sizeinbase(q_, 2);  // ~256
    int mbits  = keyBits - qbits;              // ~768 với 1024-bit p

    const int MAX_ATTEMPTS = 200000;
    bool found = false;

    for (int att = 0; att < MAX_ATTEMPTS && !found; att++) {
        // Sinh m ngẫu nhiên với mbits bit
        mpz_urandomb(m, randState_, mbits);
        mpz_setbit(m, mbits - 1); // Đặt bit cao để m có đúng mbits bit

        // p = m * q + 1
        mpz_mul(p_, m, q_);
        mpz_add_ui(p_, p_, 1);

        // Kiểm tra p có đúng keyBits bit không
        int pbits = (int)mpz_sizeinbase(p_, 2);
        if (pbits != keyBits) continue;

        // Kiểm tra nguyên tố (25 lần Miller-Rabin)
        if (mpz_probab_prime_p(p_, 25) > 0) {
            found = true;
        }
    }

    if (!found) {
        mpz_clears(temp, m, h, exp, nullptr);
        return false; // Rất hiếm xảy ra
    }

    // --------------------------------------------------------
    // Bước 3: Tính g — phần tử sinh bậc q trong Z*_p
    // g = h^((p-1)/q) mod p
    // Điều kiện: g != 1
    // --------------------------------------------------------
    mpz_sub_ui(temp, p_, 1);   // temp = p - 1
    mpz_divexact(exp, temp, q_); // exp = (p-1)/q

    do {
        // Chọn h ngẫu nhiên trong [2, p-2]
        mpz_urandomm(h, randState_, temp); // 0 <= h < p-1
        mpz_add_ui(h, h, 2);               // h >= 2
        if (mpz_cmp(h, temp) >= 0) {
            mpz_set_ui(h, 2);
        }

        // g = h^exp mod p
        mpz_powm(g_, h, exp, p_);

    } while (mpz_cmp_ui(g_, 1) == 0);

    // --------------------------------------------------------
    // Bước 4: Sinh x — khóa bí mật (0 < x < q)
    // --------------------------------------------------------
    do {
        mpz_urandomm(x_, randState_, q_);
    } while (mpz_cmp_ui(x_, 0) == 0);

    // --------------------------------------------------------
    // Bước 5: Tính y = g^x mod p — khóa công khai
    // --------------------------------------------------------
    mpz_powm(y_, g_, x_, p_);

    mpz_clears(temp, m, h, exp, nullptr);

    keyReady_ = true;
    return true;
}

// ============================================================
// NHÓM 2: Ký số thông điệp
//
// Quy trình:
// 1. H = SHA-256(message) mod q
// 2. Chọn k ngẫu nhiên (0 < k < q) — LÊN ĐỦ NGẪU NHIÊN
//    (Nếu k bị lặp lại, kẻ tấn công có thể giải ra x!)
// 3. r = (g^k mod p) mod q
// 4. s = k^(-1) * (H + x*r) mod q
// 5. Xóa k khỏi bộ nhớ ngay lập tức
// ============================================================
bool DSAModel::signMessage(const std::string& message,
                           std::string& r_out,
                           std::string& s_out)
{
    if (!keyReady_) return false;

    // Bước 1: Băm thông điệp
    mpz_t H;
    mpz_init(H);
    std::string hashHex;
    if (!sha256ToMpz(message, H, hashHex)) {
        mpz_clear(H);
        return false;
    }
    lastHash_hex_ = hashHex;

    // H mod q (đảm bảo H < q)
    mpz_mod(H, H, q_);

    mpz_t k, r, s, kinv, xr;
    mpz_inits(k, r, s, kinv, xr, nullptr);

    bool ok = false;
    // Thử lại đến khi r != 0 và s != 0 (xác suất cực thấp phải thử lại)
    for (int attempt = 0; attempt < 100; attempt++) {

        // Bước 2: Sinh k ngẫu nhiên (0 < k < q)
        // QUAN TRỌNG: k phải khác nhau mỗi lần ký!
        // Nếu dùng k giống nhau, phương trình s = k^(-1)(H+xr) mod q
        // có thể bị giải ngược để tìm x.
        do {
            mpz_urandomm(k, randState_, q_);
        } while (mpz_cmp_ui(k, 0) == 0);

        // Bước 3: r = (g^k mod p) mod q
        mpz_powm(r, g_, k, p_);   // r = g^k mod p
        mpz_mod(r, r, q_);         // r = r mod q

        if (mpz_cmp_ui(r, 0) == 0) continue; // r == 0 → thử lại

        // Bước 4: s = k^(-1) * (H + x*r) mod q
        if (mpz_invert(kinv, k, q_) == 0) continue; // k không có nghịch đảo

        mpz_mul(xr, x_, r);        // xr = x * r
        mpz_add(xr, xr, H);        // xr = H + x*r
        mpz_mod(xr, xr, q_);       // xr = (H + x*r) mod q

        mpz_mul(s, kinv, xr);      // s = k^(-1) * (H + x*r)
        mpz_mod(s, s, q_);         // s = s mod q

        if (mpz_cmp_ui(s, 0) == 0) continue; // s == 0 → thử lại

        ok = true;
        break;
    }

    if (ok) {
        r_out = mpzToHex(r);
        s_out = mpzToHex(s);
    }

    // !! Xóa k ngay lập tức sau khi ký xong (bảo mật)
    mpz_set_ui(k, 0);

    mpz_clears(H, k, r, s, kinv, xr, nullptr);
    return ok;
}

// ============================================================
// NHÓM 3: Xác minh chữ ký
//
// Quy trình:
// 1. Kiểm tra 0 < r < q và 0 < s < q
// 2. H = SHA-256(message) mod q
// 3. w = s^(-1) mod q
// 4. u1 = H * w mod q
// 5. u2 = r * w mod q
// 6. v = (g^u1 * y^u2 mod p) mod q
// 7. Hợp lệ nếu v == r
// ============================================================
bool DSAModel::verifySignature(const std::string& message,
                                const std::string& r_hex,
                                const std::string& s_hex,
                                VerifyDetails& details)
{
    if (!keyReady_) return false;

    mpz_t r, s, H, w, u1, u2, gu1, yu2, v, tmp;
    mpz_inits(r, s, H, w, u1, u2, gu1, yu2, v, tmp, nullptr);

    // Chuyển r, s từ hex sang mpz_t
    hexToMpz(r, r_hex);
    hexToMpz(s, s_hex);

    // Bước 1: Kiểm tra 0 < r < q và 0 < s < q
    if (mpz_cmp_ui(r, 0) <= 0 || mpz_cmp(r, q_) >= 0 ||
        mpz_cmp_ui(s, 0) <= 0 || mpz_cmp(s, q_) >= 0) {
        details.valid = false;
        details.hash_hex = "(Tham số r hoặc s ngoài phạm vi hợp lệ)";
        mpz_clears(r, s, H, w, u1, u2, gu1, yu2, v, tmp, nullptr);
        return false;
    }

    // Bước 2: H = SHA-256(message) mod q
    std::string hashHex;
    if (!sha256ToMpz(message, H, hashHex)) {
        mpz_clears(r, s, H, w, u1, u2, gu1, yu2, v, tmp, nullptr);
        return false;
    }
    mpz_mod(H, H, q_);
    details.hash_hex = hashHex;

    // Bước 3: w = s^(-1) mod q
    if (mpz_invert(w, s, q_) == 0) {
        details.valid = false;
        mpz_clears(r, s, H, w, u1, u2, gu1, yu2, v, tmp, nullptr);
        return false;
    }
    details.w_hex = mpzToHex(w);

    // Bước 4: u1 = H * w mod q
    mpz_mul(u1, H, w);
    mpz_mod(u1, u1, q_);
    details.u1_hex = mpzToHex(u1);

    // Bước 5: u2 = r * w mod q
    mpz_mul(u2, r, w);
    mpz_mod(u2, u2, q_);
    details.u2_hex = mpzToHex(u2);

    // Bước 6: v = (g^u1 * y^u2 mod p) mod q
    mpz_powm(gu1, g_, u1, p_);    // g^u1 mod p
    mpz_powm(yu2, y_, u2, p_);    // y^u2 mod p
    mpz_mul(v, gu1, yu2);          // g^u1 * y^u2
    mpz_mod(v, v, p_);             // mod p
    mpz_mod(v, v, q_);             // mod q
    details.v_hex = mpzToHex(v);

    // Bước 7: So sánh v và r
    details.valid = (mpz_cmp(v, r) == 0);

    mpz_clears(r, s, H, w, u1, u2, gu1, yu2, v, tmp, nullptr);
    return details.valid;
}

// ============================================================
// NHÓM 4A: Lưu khóa công khai ra file .txt
// Định dạng key=value, mỗi tham số trên một dòng
// ============================================================
bool DSAModel::savePublicKey(const std::string& filepath) const {
    if (!keyReady_) return false;

    std::ofstream f(filepath);
    if (!f.is_open()) return false;

    f << "# DSA Public Key — Nhom 9 ATBMTT\n";
    f << "# Cac tham so nay co the cong bo rong rai\n";
    f << "BITS=" << keyBits_ << "\n";
    f << "P="    << mpzToHex(p_) << "\n";
    f << "Q="    << mpzToHex(q_) << "\n";
    f << "G="    << mpzToHex(g_) << "\n";
    f << "Y="    << mpzToHex(y_) << "\n";

    return true;
}

// ============================================================
// NHÓM 4A: Lưu khóa bí mật ra file .key
// ============================================================
bool DSAModel::savePrivateKey(const std::string& filepath) const {
    if (!keyReady_) return false;

    std::ofstream f(filepath);
    if (!f.is_open()) return false;

    f << "# DSA Private Key — BAO MAT TOI DA, KHONG CHIA SE\n";
    f << "X=" << mpzToHex(x_) << "\n";

    return true;
}

// ============================================================
// NHÓM 4A: Lưu chữ ký ra file .sig
// ============================================================
bool DSAModel::saveSignature(const std::string& filepath,
                              const std::string& r_hex,
                              const std::string& s_hex) const
{
    std::ofstream f(filepath);
    if (!f.is_open()) return false;

    f << "# DSA Signature — Nhom 9 ATBMTT\n";
    f << "HASH=" << lastHash_hex_ << "\n";
    f << "R=" << r_hex << "\n";
    f << "S=" << s_hex << "\n";

    return true;
}

// ============================================================
// NHÓM 4B: Tải khóa công khai từ file .txt
// ============================================================
bool DSAModel::loadPublicKey(const std::string& filepath) {
    std::ifstream f(filepath);
    if (!f.is_open()) return false;

    std::string line;
    bool gotP = false, gotQ = false, gotG = false, gotY = false;

    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);

        if (key == "BITS") {
            keyBits_ = std::stoi(val);
        } else if (key == "P") {
            hexToMpz(p_, val);
            gotP = true;
        } else if (key == "Q") {
            hexToMpz(q_, val);
            gotQ = true;
        } else if (key == "G") {
            hexToMpz(g_, val);
            gotG = true;
        } else if (key == "Y") {
            hexToMpz(y_, val);
            gotY = true;
        }
    }

    // Xem như có cặp khóa (chỉ cần khóa công khai để xác minh)
    if (gotP && gotQ && gotG && gotY) {
        keyReady_ = true;
        return true;
    }
    return false;
}

// ============================================================
// NHÓM 4B: Tải khóa bí mật từ file .key
// ============================================================
bool DSAModel::loadPrivateKey(const std::string& filepath) {
    std::ifstream f(filepath);
    if (!f.is_open()) return false;

    std::string line;
    bool gotX = false;

    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);

        if (key == "X") {
            hexToMpz(x_, val);
            gotX = true;
        }
    }

    return gotX;
}

// ============================================================
// NHÓM 4B: Tải chữ ký từ file .sig
// ============================================================
bool DSAModel::loadSignature(const std::string& filepath,
                              std::string& r_out,
                              std::string& s_out)
{
    std::ifstream f(filepath);
    if (!f.is_open()) return false;

    std::string line;
    bool gotR = false, gotS = false;

    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);

        if (key == "R") { r_out = val; gotR = true; }
        if (key == "S") { s_out = val; gotS = true; }
        if (key == "HASH") { lastHash_hex_ = val; }
    }

    return gotR && gotS;
}

// ============================================================
// NHÓM 4C: Dọn dẹp bộ nhớ an toàn
// Xóa trắng vùng nhớ lưu khóa bí mật x
// ============================================================
void DSAModel::secureWipe() {
    // Xóa giá trị x về 0 trước khi clear
    if (keyReady_) {
        mpz_set_ui(x_, 0);
    }
}

// ============================================================
// GETTERS
// ============================================================

std::string DSAModel::getP_hex()  const { return keyReady_ ? mpzToHex(p_) : ""; }
std::string DSAModel::getQ_hex()  const { return keyReady_ ? mpzToHex(q_) : ""; }
std::string DSAModel::getG_hex()  const { return keyReady_ ? mpzToHex(g_) : ""; }
std::string DSAModel::getX_hex()  const { return keyReady_ ? mpzToHex(x_) : ""; }
std::string DSAModel::getY_hex()  const { return keyReady_ ? mpzToHex(y_) : ""; }
std::string DSAModel::getLastHash_hex() const { return lastHash_hex_; }

int DSAModel::getPBitLength() const {
    return keyReady_ ? (int)mpz_sizeinbase(p_, 2) : 0;
}
int DSAModel::getQBitLength() const {
    return keyReady_ ? (int)mpz_sizeinbase(q_, 2) : 0;
}

// ============================================================
// PRIVATE HELPERS
// ============================================================

// Chuyển mpz_t → chuỗi HEX chữ hoa
std::string DSAModel::mpzToHex(const mpz_t n) const {
    char* buf = mpz_get_str(nullptr, 16, n);
    std::string result(buf);
    free(buf);
    // Chuyển sang chữ hoa
    for (char& c : result) {
        if (c >= 'a' && c <= 'f') c -= 32;
    }
    return result;
}

// Chuyển chuỗi HEX → mpz_t
void DSAModel::hexToMpz(mpz_t n, const std::string& hex) const {
    mpz_set_str(n, hex.c_str(), 16);
}

// SHA-256 trả về chuỗi HEX 64 ký tự
bool DSAModel::sha256Hex(const std::string& message, std::string& hashOut) const {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int  hashLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return false;

    bool ok = (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) == 1) &&
              (EVP_DigestUpdate(ctx, message.c_str(), message.size()) == 1) &&
              (EVP_DigestFinal_ex(ctx, hash, &hashLen) == 1);

    EVP_MD_CTX_free(ctx);
    if (!ok) return false;

    // Chuyển bytes → hex
    std::ostringstream oss;
    for (unsigned int i = 0; i < hashLen; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << (int)(unsigned char)hash[i];
    }
    hashOut = oss.str();

    // Chuyển sang chữ hoa
    for (char& c : hashOut) {
        if (c >= 'a' && c <= 'f') c -= 32;
    }
    return true;
}

// SHA-256 trả về mpz_t (để dùng trong tính toán DSA)
bool DSAModel::sha256ToMpz(const std::string& message, mpz_t result,
                             std::string& hexOut) const
{
    unsigned char hash[32]; // SHA-256 luôn 32 byte
    unsigned int  hashLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return false;

    bool ok = (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) == 1) &&
              (EVP_DigestUpdate(ctx, message.c_str(), message.size()) == 1) &&
              (EVP_DigestFinal_ex(ctx, hash, &hashLen) == 1);

    EVP_MD_CTX_free(ctx);
    if (!ok) return false;

    // Import bytes → mpz_t (big-endian, 1 byte/limb)
    mpz_import(result, hashLen, 1, 1, 0, 0, hash);

    // Tạo hex string
    std::ostringstream oss;
    for (unsigned int i = 0; i < hashLen; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << (int)(unsigned char)hash[i];
    }
    hexOut = oss.str();
    for (char& c : hexOut) {
        if (c >= 'a' && c <= 'f') c -= 32;
    }

    return true;
}
