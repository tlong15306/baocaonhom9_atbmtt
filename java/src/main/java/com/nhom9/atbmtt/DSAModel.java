package com.nhom9.atbmtt;

import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.security.interfaces.DSAParams;
import java.security.interfaces.DSAPrivateKey;
import java.security.interfaces.DSAPublicKey;
import java.util.Base64;

/**
 * Model: Chứa logic thuật toán DSA (Digital Signature Algorithm)
 * Sử dụng thư viện java.security (JCA - Java Cryptography Architecture)
 * 
 * Thuật toán DSA dựa trên bài toán Logarit rời rạc (Discrete Logarithm Problem)
 * Tham số hệ thống: (p, q, g)
 *   - p: Số nguyên tố lớn (1024/2048/3072 bit)
 *   - q: Ước nguyên tố của (p-1), thường 160/224/256 bit
 *   - g: Phần tử sinh (generator) của nhóm con bậc q trong Z*_p
 * Cặp khóa: (x, y)
 *   - x: Khóa bí mật (private key), 0 < x < q
 *   - y: Khóa công khai (public key), y = g^x mod p
 *
 * @author Nhóm 9 - ATBMTT (Long & Vi)
 */
public class DSAModel {

    // Các tham số DSA
    private KeyPair keyPair;
    private BigInteger p, q, g; // Tham số hệ thống
    private BigInteger x;       // Khóa bí mật
    private BigInteger y;       // Khóa công khai
    private int keySize;        // Kích thước khóa (bit)
    
    // Kết quả ký số
    private byte[] lastSignatureBytes;
    private String lastSignatureBase64;
    private String lastHashHex;

    /**
     * Sinh cặp khóa DSA
     * Bước 1: Chọn số nguyên tố q (subprime)
     * Bước 2: Chọn số nguyên tố p sao cho q | (p-1)
     * Bước 3: Tính g = h^((p-1)/q) mod p, với h là số ngẫu nhiên
     * Bước 4: Chọn x ngẫu nhiên, 0 < x < q (khóa bí mật)
     * Bước 5: Tính y = g^x mod p (khóa công khai)
     *
     * @param keySize kích thước khóa (1024, 2048, hoặc 3072)
     * @throws NoSuchAlgorithmException nếu thuật toán không được hỗ trợ
     */
    public void generateKeyPair(int keySize) throws NoSuchAlgorithmException {
        this.keySize = keySize;
        
        // Sử dụng KeyPairGenerator với thuật toán DSA
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("DSA");
        
        // SecureRandom: Sinh số ngẫu nhiên an toàn (CSPRNG)
        // Đảm bảo tham số k không bị lặp lại hoặc dự đoán được
        SecureRandom secureRandom = SecureRandom.getInstanceStrong();
        keyGen.initialize(keySize, secureRandom);
        
        this.keyPair = keyGen.generateKeyPair();
        
        // Trích xuất các tham số hệ thống (p, q, g)
        DSAPublicKey pubKey = (DSAPublicKey) keyPair.getPublic();
        DSAPrivateKey privKey = (DSAPrivateKey) keyPair.getPrivate();
        DSAParams params = pubKey.getParams();
        
        this.p = params.getP();
        this.q = params.getQ();
        this.g = params.getG();
        this.y = pubKey.getY();  // y = g^x mod p
        this.x = privKey.getX(); // Khóa bí mật
    }

    /**
     * Ký số thông điệp bằng thuật toán SHA256withDSA
     * 
     * Quy trình ký:
     * 1. Băm thông điệp: H(m) = SHA-256(message)
     * 2. Chọn k ngẫu nhiên, 0 < k < q (do SecureRandom đảm bảo)
     * 3. Tính r = (g^k mod p) mod q
     * 4. Tính s = k^(-1) * (H(m) + x*r) mod q
     * 5. Chữ ký số là cặp (r, s)
     *
     * Lưu ý: Nếu k bị lặp lại, kẻ tấn công có thể giải ngược khóa bí mật x
     * bằng phương trình: s = k^(-1)(H(m) + xr) mod q
     *
     * @param message thông điệp cần ký
     * @return chuỗi chữ ký số dạng Base64
     * @throws Exception nếu có lỗi trong quá trình ký
     */
    public String signMessage(String message) throws Exception {
        if (keyPair == null) {
            throw new IllegalStateException("Chưa sinh cặp khóa! Vui lòng sinh khóa trước.");
        }
        
        // Tính hash SHA-256 của thông điệp
        MessageDigest sha256 = MessageDigest.getInstance("SHA-256");
        byte[] hashBytes = sha256.digest(message.getBytes(StandardCharsets.UTF_8));
        this.lastHashHex = bytesToHex(hashBytes);
        
        // Sử dụng Signature API với thuật toán SHA256withDSA
        // SHA-256 được chọn thay vì SHA-1 vì SHA-1 đã bị coi là lỗi thời
        // và có nguy cơ bị tấn công đụng độ (collision attack)
        Signature dsa = Signature.getInstance("SHA256withDSA");
        dsa.initSign(keyPair.getPrivate());
        dsa.update(message.getBytes(StandardCharsets.UTF_8));
        
        this.lastSignatureBytes = dsa.sign();
        this.lastSignatureBase64 = Base64.getEncoder().encodeToString(lastSignatureBytes);
        
        return lastSignatureBase64;
    }

    /**
     * Xác minh chữ ký số
     * 
     * Quy trình xác minh:
     * 1. Tính w = s^(-1) mod q
     * 2. Tính u1 = H(m) * w mod q
     * 3. Tính u2 = r * w mod q
     * 4. Tính v = (g^u1 * y^u2 mod p) mod q
     * 5. Chữ ký hợp lệ nếu v == r
     *
     * @param message thông điệp gốc
     * @param signatureBase64 chữ ký số dạng Base64
     * @return true nếu chữ ký hợp lệ, false nếu không
     * @throws Exception nếu có lỗi trong quá trình xác minh
     */
    public boolean verifySignature(String message, String signatureBase64) throws Exception {
        if (keyPair == null) {
            throw new IllegalStateException("Chưa có khóa công khai! Vui lòng sinh khóa trước.");
        }
        
        byte[] sigBytes = Base64.getDecoder().decode(signatureBase64);
        
        Signature dsa = Signature.getInstance("SHA256withDSA");
        dsa.initVerify(keyPair.getPublic());
        dsa.update(message.getBytes(StandardCharsets.UTF_8));
        
        return dsa.verify(sigBytes);
    }

    /**
     * Chuyển đổi mảng byte sang chuỗi hex
     */
    private String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    // === Getters cho Controller truy xuất dữ liệu ===
    
    public BigInteger getP() { return p; }
    public BigInteger getQ() { return q; }
    public BigInteger getG() { return g; }
    public BigInteger getX() { return x; }
    public BigInteger getY() { return y; }
    public int getKeySize() { return keySize; }
    public String getLastSignatureBase64() { return lastSignatureBase64; }
    public byte[] getLastSignatureBytes() { return lastSignatureBytes; }
    public String getLastHashHex() { return lastHashHex; }
    public KeyPair getKeyPair() { return keyPair; }
    
    /**
     * Kiểm tra xem đã có cặp khóa chưa
     */
    public boolean hasKeyPair() {
        return keyPair != null;
    }
}
