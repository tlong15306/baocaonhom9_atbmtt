/*
 * dsa_model.cpp — Triển khai thuật toán DSA tự viết bằng GMP + OpenSSL
 * Nhóm 9 — An Toàn Bảo Mật Thông Tin, 2025-2026
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

DSAModel::DSAModel() : randInit_(false), keyBits_(0), keyReady_(false) {
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

void DSAModel::initRandom() {
    if (randInit_) {
        gmp_randclear(randState_);
    }

    gmp_randinit_mt(randState_);

    unsigned char seedBytes[32];
    if (RAND_bytes(seedBytes, sizeof(seedBytes)) != 1) {
        unsigned long t = (unsigned long)time(nullptr);
        gmp_randseed_ui(randState_, t);
    } else {
        mpz_t mpzSeed;
        mpz_init(mpzSeed);
        mpz_import(mpzSeed, sizeof(seedBytes), 1, 1, 0, 0, seedBytes);
        gmp_randseed(randState_, mpzSeed);
        mpz_clear(mpzSeed);
        OPENSSL_cleanse(seedBytes, sizeof(seedBytes));
    }

    randInit_ = true;
}

bool DSAModel::generateKeyPair(int keyBits) {
    keyBits_  = keyBits;
    keyReady_ = false;

    initRandom();

    mpz_t temp, m, h, exp;
    mpz_inits(temp, m, h, exp, nullptr);

    do {
        mpz_urandomb(q_, randState_, 256);
        mpz_setbit(q_, 255);
        mpz_setbit(q_, 0);
    } while (mpz_probab_prime_p(q_, 50) == 0);

    int qbits  = (int)mpz_sizeinbase(q_, 2);
    int mbits  = keyBits - qbits;

    const int MAX_ATTEMPTS = 200000;
    bool found = false;

    for (int att = 0; att < MAX_ATTEMPTS && !found; att++) {
        mpz_urandomb(m, randState_, mbits);
        mpz_setbit(m, mbits - 1);

        mpz_mul(p_, m, q_);
        mpz_add_ui(p_, p_, 1);

        int pbits = (int)mpz_sizeinbase(p_, 2);
        if (pbits != keyBits) continue;

        if (mpz_probab_prime_p(p_, 25) > 0) {
            found = true;
        }
    }

    if (!found) {
        mpz_clears(temp, m, h, exp, nullptr);
        return false;
    }

    mpz_sub_ui(temp, p_, 1);
    mpz_divexact(exp, temp, q_);

    do {
        mpz_urandomm(h, randState_, temp);
        mpz_add_ui(h, h, 2);
        if (mpz_cmp(h, temp) >= 0) {
            mpz_set_ui(h, 2);
        }

        mpz_powm(g_, h, exp, p_);

    } while (mpz_cmp_ui(g_, 1) == 0);

    do {
        mpz_urandomm(x_, randState_, q_);
    } while (mpz_cmp_ui(x_, 0) == 0);

    mpz_powm(y_, g_, x_, p_);

    mpz_clears(temp, m, h, exp, nullptr);

    keyReady_ = true;
    return true;
}

bool DSAModel::signMessage(const std::string& message,
                           std::string& r_out,
                           std::string& s_out)
{
    if (!keyReady_) return false;

    mpz_t H;
    mpz_init(H);
    std::string hashHex;
    if (!sha256ToMpz(message, H, hashHex)) {
        mpz_clear(H);
        return false;
    }
    lastHash_hex_ = hashHex;

    mpz_mod(H, H, q_);

    mpz_t k, r, s, kinv, xr;
    mpz_inits(k, r, s, kinv, xr, nullptr);

    bool ok = false;
    for (int attempt = 0; attempt < 100; attempt++) {

        do {
            mpz_urandomm(k, randState_, q_);
        } while (mpz_cmp_ui(k, 0) == 0);

        mpz_powm(r, g_, k, p_);
        mpz_mod(r, r, q_);

        if (mpz_cmp_ui(r, 0) == 0) continue;

        if (mpz_invert(kinv, k, q_) == 0) continue;

        mpz_mul(xr, x_, r);
        mpz_add(xr, xr, H);
        mpz_mod(xr, xr, q_);

        mpz_mul(s, kinv, xr);
        mpz_mod(s, s, q_);

        if (mpz_cmp_ui(s, 0) == 0) continue;

        ok = true;
        break;
    }

    if (ok) {
        r_out = mpzToHex(r);
        s_out = mpzToHex(s);
    }

    mpz_set_ui(k, 0);

    mpz_clears(H, k, r, s, kinv, xr, nullptr);
    return ok;
}

bool DSAModel::verifySignature(const std::string& message,
                               const std::string& r_hex,
                               const std::string& s_hex,
                               const std::string& y_hex,
                               VerifyDetails& details)
{
    if (!keyReady_) return false;

    mpz_t r, s, y_pub, H, w, u1, u2, gu1, yu2, v, tmp;
    mpz_inits(r, s, y_pub, H, w, u1, u2, gu1, yu2, v, tmp, nullptr);

    if (mpz_set_str(r, r_hex.c_str(), 16) == -1 ||
        mpz_set_str(s, s_hex.c_str(), 16) == -1 ||
        mpz_set_str(y_pub, y_hex.c_str(), 16) == -1) {
        details.valid = false;
        details.hash_hex = "(Lỗi định dạng cấu trúc HEX)";
        details.w_hex = "—";
        details.u1_hex = "—";
        details.u2_hex = "—";
        details.v_hex = "—";
        mpz_clears(r, s, y_pub, H, w, u1, u2, gu1, yu2, v, tmp, nullptr);
        return false;
    }

    if (mpz_cmp_ui(r, 0) <= 0 || mpz_cmp(r, q_) >= 0 ||
        mpz_cmp_ui(s, 0) <= 0 || mpz_cmp(s, q_) >= 0) {
        details.valid = false;
        details.hash_hex = "(Tham số r hoặc s bị lỗi ngoài phạm vi)";
        mpz_clears(r, s, y_pub, H, w, u1, u2, gu1, yu2, v, tmp, nullptr);
        return false;
    }

    std::string hashHex;
    if (!sha256ToMpz(message, H, hashHex)) {
        mpz_clears(r, s, y_pub, H, w, u1, u2, gu1, yu2, v, tmp, nullptr);
        return false;
    }
    mpz_mod(H, H, q_);
    details.hash_hex = hashHex;

    if (mpz_invert(w, s, q_) == 0) {
        details.valid = false;
        mpz_clears(r, s, y_pub, H, w, u1, u2, gu1, yu2, v, tmp, nullptr);
        return false;
    }
    details.w_hex = mpzToHex(w);

    mpz_mul(u1, H, w);
    mpz_mod(u1, u1, q_);
    details.u1_hex = mpzToHex(u1);

    mpz_mul(u2, r, w);
    mpz_mod(u2, u2, q_);
    details.u2_hex = mpzToHex(u2);

    mpz_powm(gu1, g_, u1, p_);
    mpz_powm(yu2, y_pub, u2, p_);
    mpz_mul(v, gu1, yu2);
    mpz_mod(v, v, p_);
    mpz_mod(v, v, q_);
    details.v_hex = mpzToHex(v);

    details.valid = (mpz_cmp(v, r) == 0);

    mpz_clears(r, s, y_pub, H, w, u1, u2, gu1, yu2, v, tmp, nullptr);
    return details.valid;
}

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

bool DSAModel::savePrivateKey(const std::string& filepath) const {
    if (!keyReady_) return false;

    std::ofstream f(filepath);
    if (!f.is_open()) return false;

    f << "# DSA Private Key — BAO MAT TOI DA, KHONG CHIA SE\n";
    f << "X=" << mpzToHex(x_) << "\n";

    return true;
}

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

    if (gotP && gotQ && gotG && gotY) {
        keyReady_ = true;
        return true;
    }
    return false;
}

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

void DSAModel::secureWipe() {
    if (keyReady_) {
        mpz_set_ui(x_, 0);
    }
}

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

std::string DSAModel::mpzToHex(const mpz_t n) const {
    char* buf = mpz_get_str(nullptr, 16, n);
    std::string result(buf);
    
    // SỬA DÒNG NÀY: Dùng bộ hủy giải phóng RAM chuẩn của thư viện GMP
    void (*freefunc)(void *, size_t);
    mp_get_memory_functions(nullptr, nullptr, &freefunc);
    freefunc(buf, strlen(buf) + 1);

    for (char& c : result) {
        if (c >= 'a' && c <= 'f') c -= 32;
    }
    return result;
}

void DSAModel::hexToMpz(mpz_t n, const std::string& hex) const {
    mpz_set_str(n, hex.c_str(), 16);
}

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

    std::ostringstream oss;
    for (unsigned int i = 0; i < hashLen; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0')
        << (int)(unsigned char)hash[i];
    }
    hashOut = oss.str();

    for (char& c : hashOut) {
        if (c >= 'a' && c <= 'f') c -= 32;
    }
    return true;
}

bool DSAModel::sha256ToMpz(const std::string& message, mpz_t result,
                           std::string& hexOut) const
{
    unsigned char hash[32];
    unsigned int  hashLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return false;

    bool ok = (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) == 1) &&
              (EVP_DigestUpdate(ctx, message.c_str(), message.size()) == 1) &&
              (EVP_DigestFinal_ex(ctx, hash, &hashLen) == 1);

    EVP_MD_CTX_free(ctx);
    if (!ok) return false;

    mpz_import(result, hashLen, 1, 1, 0, 0, hash);

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