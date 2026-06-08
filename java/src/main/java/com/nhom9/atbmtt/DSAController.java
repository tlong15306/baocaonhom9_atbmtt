package com.nhom9.atbmtt;

import javafx.animation.*;
import javafx.application.Platform;
import javafx.concurrent.Task;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.util.Duration;
import javafx.stage.FileChooser;
import javafx.stage.Stage;

import java.io.File;
import java.io.PrintWriter;
import java.math.BigInteger;
import java.net.URL;
import java.security.interfaces.DSAPrivateKey;
import java.security.interfaces.DSAPublicKey;
import java.util.Base64;
import java.util.ResourceBundle;

/**
 * Controller: Điều khiển luồng dữ liệu giữa View (FXML) và Model (DSAModel)
 * Xử lý các sự kiện từ giao diện người dùng
 *
 * @author Nhóm 9 - ATBMTT (Long & Vi)
 */
public class DSAController implements Initializable {

    // === Model ===
    private final DSAModel dsaModel = new DSAModel();
    private String actualPrivateKeyHex = "";
    private boolean isPrivateKeyVisible = false;

    // === FXML UI Components ===
    
    // Tab Sinh Khóa
    @FXML private ComboBox<String> keySizeCombo;
    @FXML private TextArea paramPArea;
    @FXML private TextArea paramQArea;
    @FXML private TextArea paramGArea;
    @FXML private TextArea privateKeyArea;
    @FXML private Button togglePrivateKeyBtn;
    @FXML private TextArea publicKeyArea;
    @FXML private Label keyStatusLabel;
    @FXML private ProgressIndicator keyProgress;
    @FXML private VBox keyResultBox;
    @FXML private Label pBitLengthLabel;
    @FXML private Label qBitLengthLabel;
    
    // Tab Ký Số
    @FXML private TextArea messageArea;
    @FXML private TextArea signatureArea;
    @FXML private TextArea hashArea;
    @FXML private Label signStatusLabel;
    @FXML private Button signButton;
    @FXML private VBox sigDetailsBox;
    @FXML private Button toggleSigDetailsBtn;
    @FXML private TextArea sigRAreaHex;
    @FXML private TextArea sigRAreaDec;
    @FXML private TextArea sigSAreaHex;
    @FXML private TextArea sigSAreaDec;
    private boolean isSigDetailsVisible = false;
    
    // Tab Xác Minh
    @FXML private TextArea verifyMessageArea;
    @FXML private TextArea verifySignatureArea;
    @FXML private Label verifyStatusLabel;
    @FXML private VBox verifyResultBox;
    @FXML private Label verifyResultLabel;
    @FXML private Circle verifyIndicator;
    @FXML private Button verifyButton;
    
    // Tab Ký Số mới
    @FXML private Label signKeyStatusLabel;
    @FXML private VBox signKeyDetailsBox;
    @FXML private Button toggleSignKeyDetailsBtn;
    @FXML private TextArea signPrivateKeyArea;
    @FXML private Button toggleSignPrivateKeyBtn;
    @FXML private TextArea signParamPArea;
    @FXML private TextArea signParamQArea;
    @FXML private TextArea signParamGArea;

    // Tab Xác Minh mới
    @FXML private Label verifyKeyStatusLabel;
    @FXML private VBox verifyKeyDetailsBox;
    @FXML private Button toggleVerifyKeyDetailsBtn;
    @FXML private TextArea verifyPublicKeyArea;
    @FXML private TextArea verifyParamPArea;
    @FXML private TextArea verifyParamQArea;
    @FXML private TextArea verifyParamGArea;

    private boolean isSignKeyDetailsVisible = false;
    private boolean isVerifyKeyDetailsVisible = false;
    private boolean isSignPrivateKeyVisible = false;

    // Main
    @FXML private TabPane mainTabPane;
    @FXML private Label footerLabel;

    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // Thiết lập giá trị cho ComboBox kích thước khóa
        keySizeCombo.getItems().addAll("1024 bit", "2048 bit");
        keySizeCombo.setValue("1024 bit");
        
        // Ẩn kết quả ban đầu
        keyResultBox.setVisible(false);
        keyResultBox.setManaged(false);
        keyProgress.setVisible(false);
        
        if (verifyResultBox != null) {
            verifyResultBox.setVisible(false);
            verifyResultBox.setManaged(false);
        }
        
        // Cập nhật footer
        footerLabel.setText("Nhóm 9 - An Toàn Bảo Mật Thông Tin | DSA Digital Signature Demo | 2025-2026");
    }

    /**
     * Xử lý sự kiện nhấn nút "Sinh Khóa"
     * Chạy trên luồng riêng để không block giao diện
     */
    @FXML
    private void handleGenerateKeys() {
        String selected = keySizeCombo.getValue();
        int keySize = Integer.parseInt(selected.split(" ")[0]);
        
        // Hiển thị progress
        keyProgress.setVisible(true);
        keyStatusLabel.setText("Đang sinh khóa DSA " + keySize + " bit...");
        keyStatusLabel.setStyle("-fx-text-fill: #f59e0b;");
        keyResultBox.setVisible(false);
        keyResultBox.setManaged(false);
        
        // Chạy trên luồng nền để không đóng băng giao diện
        Task<Void> task = new Task<>() {
            @Override
            protected Void call() throws Exception {
                dsaModel.generateKeyPair(keySize);
                return null;
            }
        };
        
        task.setOnSucceeded(e -> {
            keyProgress.setVisible(false);
            keyStatusLabel.setText("Sinh khóa thành công! Kích thước: " + keySize + " bit");
            keyStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
            
            // Hiển thị các tham số
            BigInteger p = dsaModel.getP();
            BigInteger q = dsaModel.getQ();
            BigInteger g = dsaModel.getG();
            
            paramPArea.setText(p.toString(16).toUpperCase());
            paramQArea.setText(q.toString(16).toUpperCase());
            paramGArea.setText(g.toString(16).toUpperCase());
            
            actualPrivateKeyHex = dsaModel.getX().toString(16).toUpperCase();
            if (isPrivateKeyVisible) {
                privateKeyArea.setText(actualPrivateKeyHex);
                if (togglePrivateKeyBtn != null) togglePrivateKeyBtn.setText("Ẩn");
            } else {
                privateKeyArea.setText("•".repeat(Math.min(actualPrivateKeyHex.length(), 64)));
                if (togglePrivateKeyBtn != null) togglePrivateKeyBtn.setText("Hiện");
            }
            
            publicKeyArea.setText(dsaModel.getY().toString(16).toUpperCase());
            
            pBitLengthLabel.setText("Độ dài: " + p.bitLength() + " bit");
            qBitLengthLabel.setText("Độ dài: " + q.bitLength() + " bit");
            
            if (signKeyStatusLabel != null) {
                signKeyStatusLabel.setText("Đang sử dụng khóa vừa sinh ở Tab 1 (" + keySize + " bit)");
                signKeyStatusLabel.setStyle("-fx-text-fill: #34d399;");
            }
            if (verifyKeyStatusLabel != null) {
                verifyKeyStatusLabel.setText("Đang sử dụng khóa vừa sinh ở Tab 1 (" + keySize + " bit)");
                verifyKeyStatusLabel.setStyle("-fx-text-fill: #34d399;");
            }
            
            updateKeyDetailsUI();
            
            // Hien ket qua voi animation slide-in premium
            keyResultBox.setVisible(true);
            keyResultBox.setManaged(true);
            keyResultBox.setOpacity(0);
            keyResultBox.setTranslateY(18);
            
            FadeTransition fade = new FadeTransition(Duration.millis(450), keyResultBox);
            fade.setFromValue(0);
            fade.setToValue(1);
            fade.setInterpolator(javafx.animation.Interpolator.EASE_OUT);
            
            TranslateTransition slide = new TranslateTransition(Duration.millis(450), keyResultBox);
            slide.setFromY(18);
            slide.setToY(0);
            slide.setInterpolator(javafx.animation.Interpolator.SPLINE(0.16, 1, 0.3, 1));
            
            ParallelTransition parallel = new ParallelTransition(fade, slide);
            parallel.play();
        });
        
        task.setOnFailed(e -> {
            keyProgress.setVisible(false);
            keyStatusLabel.setText("Lỗi sinh khóa: " + task.getException().getMessage());
            keyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
        });
        
        new Thread(task).start();
    }

    /**
     * Xử lý sự kiện nhấn nút "Ký Số"
     * Tương ứng với hàm handleSignAction trong báo cáo
     */
    @FXML
    private void handleSignAction() {
        try {
            String message = messageArea.getText();
            
            if (message == null || message.trim().isEmpty()) {
                signStatusLabel.setText("Vui lòng nhập thông điệp cần ký!");
                signStatusLabel.setStyle("-fx-text-fill: #f59e0b;");
                return;
            }
            
            if (!dsaModel.hasKeyPair()) {
                signStatusLabel.setText("Chưa có cặp khóa! Vui lòng sinh khóa trước (Tab 1).");
                signStatusLabel.setStyle("-fx-text-fill: #f59e0b;");
                
                // Chuyển sang tab sinh khóa
                mainTabPane.getSelectionModel().select(0);
                return;
            }
            
            // Thực hiện ký số
            String signatureBase64 = dsaModel.signMessage(message);
            
            // Hiển thị kết quả
            signatureArea.setText(signatureBase64);
            hashArea.setText(dsaModel.getLastHashHex().toUpperCase());
            
            signStatusLabel.setText("Thông điệp đã được ký thành công bằng SHA256withDSA!");
            signStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
            
            // Tự động điền vào tab Xác minh
            verifyMessageArea.setText(message);
            verifySignatureArea.setText(signatureBase64);
            
            // Cập nhật chi tiết chữ ký (r, s) nếu đang hiển thị
            if (isSigDetailsVisible) {
                updateSigDetailsUI();
            }
            
            // Animation hieu ung ket qua premium
            FadeTransition fade = new FadeTransition(Duration.millis(350), signatureArea);
            fade.setFromValue(0.2);
            fade.setToValue(1);
            fade.setInterpolator(javafx.animation.Interpolator.EASE_OUT);
            fade.play();
            
        } catch (Exception e) {
            signStatusLabel.setText("Lỗi ký số: " + e.getMessage());
            signStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
        }
    }

    /**
     * Xử lý sự kiện nhấn nút "Xác Minh"
     * Kiểm tra tính hợp lệ của chữ ký dựa trên khóa công khai y
     */
    @FXML
    private void handleVerifyAction() {
        // Reset verify result box visibility
        verifyResultBox.setVisible(false);
        verifyResultBox.setManaged(false);

        String message = verifyMessageArea.getText();
        String signatureInput = verifySignatureArea.getText();
        
        // 1. Kiểm tra thông điệp trống
        if (message == null || message.trim().isEmpty()) {
            verifyStatusLabel.setText("Lỗi: Vui lòng nhập thông điệp cần xác minh!");
            verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            return;
        }
        
        // 2. Kiểm tra chữ ký trống
        if (signatureInput == null || signatureInput.trim().isEmpty()) {
            verifyStatusLabel.setText("Lỗi: Vui lòng nhập chữ ký số cần xác minh!");
            verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            return;
        }
        
        // 3. Kiểm tra sự tồn tại của khóa công khai
        if (!dsaModel.hasKeyPair() || dsaModel.getKeyPair().getPublic() == null) {
            verifyStatusLabel.setText("Lỗi: Chưa có khóa công khai! Vui lòng sinh khóa (Tab 1) hoặc tải khóa công khai (Tab 3).");
            verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            return;
        }

        // Làm sạch chuỗi chữ ký (nếu có bọc tag PEM)
        String signature = signatureInput.trim();
        if (signature.contains("-----BEGIN DSA SIGNATURE-----")) {
            int start = signature.indexOf("-----BEGIN DSA SIGNATURE-----") + "-----BEGIN DSA SIGNATURE-----".length();
            int end = signature.indexOf("-----END DSA SIGNATURE-----");
            if (end > start) {
                signature = signature.substring(start, end).trim();
            }
        }
        
        try {
            // 4. Kiểm tra giải mã Base64
            byte[] sigBytes;
            try {
                sigBytes = Base64.getDecoder().decode(signature);
            } catch (IllegalArgumentException e) {
                verifyResultBox.setVisible(true);
                verifyResultBox.setManaged(true);
                verifyResultLabel.setText("LỖI ĐỊNH DẠNG");
                verifyResultLabel.setStyle("-fx-text-fill: #f59e0b; -fx-font-weight: bold; -fx-font-size: 16px;");
                verifyIndicator.setStyle("-fx-fill: #f59e0b;");
                verifyStatusLabel.setText("Lỗi: Chữ ký không đúng định dạng Base64! Vui lòng kiểm tra lại chuỗi chữ ký.");
                verifyStatusLabel.setStyle("-fx-text-fill: #f59e0b;");
                return;
            }

            // 5. Kiểm tra cấu trúc ASN.1 DER của chữ ký DSA
            try {
                decodeDSASignature(sigBytes);
            } catch (Exception e) {
                verifyResultBox.setVisible(true);
                verifyResultBox.setManaged(true);
                verifyResultLabel.setText("SAI CẤU TRÚC");
                verifyResultLabel.setStyle("-fx-text-fill: #f59e0b; -fx-font-weight: bold; -fx-font-size: 16px;");
                verifyIndicator.setStyle("-fx-fill: #f59e0b;");
                verifyStatusLabel.setText("Lỗi: Chữ ký đúng định dạng Base64 nhưng sai cấu trúc DSA (DER SEQUENCE {r, s})!");
                verifyStatusLabel.setStyle("-fx-text-fill: #f59e0b;");
                return;
            }

            // 6. Thực hiện xác minh chữ ký số về mặt toán học
            boolean isValid = dsaModel.verifySignature(message, signature);
            
            verifyResultBox.setVisible(true);
            verifyResultBox.setManaged(true);
            
            if (isValid) {
                verifyResultLabel.setText("CHỮ KÝ HỢP LỆ");
                verifyResultLabel.setStyle("-fx-text-fill: #2dd4a0; -fx-font-weight: bold; -fx-font-size: 16px;");
                verifyIndicator.setStyle("-fx-fill: #2dd4a0;");
                verifyStatusLabel.setText("Xác minh thành công! Thông điệp nguyên vẹn và khớp với khóa công khai.");
                verifyStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
            } else {
                verifyResultLabel.setText("CHỮ KÝ KHÔNG HỢP LỆ");
                verifyResultLabel.setStyle("-fx-text-fill: #f43f5e; -fx-font-weight: bold; -fx-font-size: 16px;");
                verifyIndicator.setStyle("-fx-fill: #f43f5e;");
                verifyStatusLabel.setText(
                    "Cảnh báo: Xác minh thất bại! Có 2 khả năng xảy ra:\n" +
                    "• 1. Tính toàn vẹn bị vi phạm: Nội dung thông điệp đã bị sửa đổi (dù chỉ một ký tự hoặc khoảng trắng) so với lúc ký.\n" +
                    "• 2. Sai lệch nguồn gốc: Chữ ký này được tạo ra từ một khóa bí mật khác, không khớp với khóa công khai đang dùng để đối chiếu."
                );
                verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e; -fx-font-size: 12px;");
            }
            
            // Hieu ung overshoot spring-like (JavaFX SPLINE requires [0,1])
            verifyResultBox.setOpacity(0);
            verifyResultBox.setScaleX(0.85);
            verifyResultBox.setScaleY(0.85);
            
            FadeTransition fadeIn = new FadeTransition(Duration.millis(300), verifyResultBox);
            fadeIn.setFromValue(0);
            fadeIn.setToValue(1);
            fadeIn.setInterpolator(javafx.animation.Interpolator.EASE_OUT);
            
            // Overshoot: scale 0.85 -> 1.06 -> 1.0
            ScaleTransition scaleUp = new ScaleTransition(Duration.millis(350), verifyResultBox);
            scaleUp.setFromX(0.85);
            scaleUp.setFromY(0.85);
            scaleUp.setToX(1.06);
            scaleUp.setToY(1.06);
            scaleUp.setInterpolator(javafx.animation.Interpolator.EASE_OUT);
            
            ScaleTransition scaleSettle = new ScaleTransition(Duration.millis(200), verifyResultBox);
            scaleSettle.setFromX(1.06);
            scaleSettle.setFromY(1.06);
            scaleSettle.setToX(1.0);
            scaleSettle.setToY(1.0);
            scaleSettle.setInterpolator(javafx.animation.Interpolator.EASE_BOTH);
            
            SequentialTransition bounce = new SequentialTransition(scaleUp, scaleSettle);
            ParallelTransition bounceIn = new ParallelTransition(fadeIn, bounce);
            bounceIn.play();
            
        } catch (java.security.InvalidKeyException e) {
            verifyResultBox.setVisible(true);
            verifyResultBox.setManaged(true);
            verifyResultLabel.setText("LỖI KHÓA");
            verifyResultLabel.setStyle("-fx-text-fill: #f43f5e; -fx-font-weight: bold; -fx-font-size: 16px;");
            verifyIndicator.setStyle("-fx-fill: #f43f5e;");
            verifyStatusLabel.setText("Lỗi: Khóa công khai hiện tại không hợp lệ cho thuật toán DSA!");
            verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
        } catch (java.security.SignatureException e) {
            verifyResultBox.setVisible(true);
            verifyResultBox.setManaged(true);
            verifyResultLabel.setText("LỖI XÁC MINH");
            verifyResultLabel.setStyle("-fx-text-fill: #f43f5e; -fx-font-weight: bold; -fx-font-size: 16px;");
            verifyIndicator.setStyle("-fx-fill: #f43f5e;");
            verifyStatusLabel.setText("Lỗi xử lý chữ ký: " + e.getMessage());
            verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
        } catch (Exception e) {
            verifyResultBox.setVisible(true);
            verifyResultBox.setManaged(true);
            verifyResultLabel.setText("LỖI HỆ THỐNG");
            verifyResultLabel.setStyle("-fx-text-fill: #f43f5e; -fx-font-weight: bold; -fx-font-size: 16px;");
            verifyIndicator.setStyle("-fx-fill: #f43f5e;");
            verifyStatusLabel.setText("Lỗi không xác định: " + e.getMessage());
            verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
        }
    }

    /**
     * Xóa nội dung tab Ký số
     */
    @FXML
    private void handleClearSign() {
        messageArea.clear();
        signatureArea.clear();
        hashArea.clear();
        signStatusLabel.setText("");
        if (sigDetailsBox != null) {
            sigDetailsBox.setVisible(false);
            sigDetailsBox.setManaged(false);
        }
        isSigDetailsVisible = false;
        if (toggleSigDetailsBtn != null) {
            toggleSigDetailsBtn.setText("Tách Chữ Ký (r, s)");
        }
        if (sigRAreaHex != null) sigRAreaHex.clear();
        if (sigRAreaDec != null) sigRAreaDec.clear();
        if (sigSAreaHex != null) sigSAreaHex.clear();
        if (sigSAreaDec != null) sigSAreaDec.clear();
    }

    /**
     * Xóa nội dung tab Xác minh
     */
    @FXML
    private void handleClearVerify() {
        verifyMessageArea.clear();
        verifySignatureArea.clear();
        verifyStatusLabel.setText("");
        if (verifyResultBox != null) {
            verifyResultBox.setVisible(false);
            verifyResultBox.setManaged(false);
        }
    }
    
    /**
     * Sao chép chữ ký vào clipboard
     */
    @FXML
    private void handleCopySignature() {
        if (signatureArea.getText() != null && !signatureArea.getText().isEmpty()) {
            javafx.scene.input.Clipboard clipboard = javafx.scene.input.Clipboard.getSystemClipboard();
            javafx.scene.input.ClipboardContent content = new javafx.scene.input.ClipboardContent();
            content.putString(signatureArea.getText());
            clipboard.setContent(content);
            signStatusLabel.setText("Đã sao chép chữ ký vào clipboard!");
            signStatusLabel.setStyle("-fx-text-fill: #38bdf8;");
        }
    }

    /**
     * Sao chép thông điệp ký vào clipboard
     */
    @FXML
    private void handleCopyMessage() {
        if (messageArea.getText() != null && !messageArea.getText().isEmpty()) {
            javafx.scene.input.Clipboard clipboard = javafx.scene.input.Clipboard.getSystemClipboard();
            javafx.scene.input.ClipboardContent content = new javafx.scene.input.ClipboardContent();
            content.putString(messageArea.getText());
            clipboard.setContent(content);
            signStatusLabel.setText("Đã sao chép thông điệp vào clipboard!");
            signStatusLabel.setStyle("-fx-text-fill: #38bdf8;");
        } else {
            signStatusLabel.setText("Chưa có thông điệp để sao chép!");
            signStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
        }
    }

    /**
     * Sao chép thông điệp cần xác minh vào clipboard
     */
    @FXML
    private void handleCopyVerifyMessage() {
        if (verifyMessageArea.getText() != null && !verifyMessageArea.getText().isEmpty()) {
            javafx.scene.input.Clipboard clipboard = javafx.scene.input.Clipboard.getSystemClipboard();
            javafx.scene.input.ClipboardContent content = new javafx.scene.input.ClipboardContent();
            content.putString(verifyMessageArea.getText());
            clipboard.setContent(content);
            verifyStatusLabel.setText("Đã sao chép thông điệp xác minh vào clipboard!");
            verifyStatusLabel.setStyle("-fx-text-fill: #38bdf8;");
        } else {
            verifyStatusLabel.setText("Chưa có thông điệp xác minh để sao chép!");
            verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
        }
    }

    /**
     * Lưu khóa bí mật ra file
     */
    @FXML
    private void handleSavePrivateKey() {
        if (!dsaModel.hasKeyPair()) {
            keyStatusLabel.setText("Chưa có cặp khóa! Vui lòng sinh khóa trước.");
            keyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            return;
        }
        
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Lưu Khóa Bí Mật DSA");
        fileChooser.setInitialFileName("dsa_private_key.txt");
        fileChooser.getExtensionFilters().addAll(
            new FileChooser.ExtensionFilter("Text Files (*.txt)", "*.txt"),
            new FileChooser.ExtensionFilter("Key Files (*.key)", "*.key"),
            new FileChooser.ExtensionFilter("All Files (*.*)", "*.*")
        );
        
        Stage stage = (Stage) privateKeyArea.getScene().getWindow();
        File file = fileChooser.showSaveDialog(stage);
        
        if (file != null) {
            try (PrintWriter writer = new PrintWriter(file)) {
                DSAPrivateKey privKey = (DSAPrivateKey) dsaModel.getKeyPair().getPrivate();
                String base64Key = Base64.getEncoder().encodeToString(privKey.getEncoded());
                
                writer.println("-----BEGIN DSA PRIVATE KEY-----");
                writer.println("p: " + dsaModel.getP().toString(16).toUpperCase());
                writer.println("q: " + dsaModel.getQ().toString(16).toUpperCase());
                writer.println("g: " + dsaModel.getG().toString(16).toUpperCase());
                writer.println("x: " + dsaModel.getX().toString(16).toUpperCase());
                writer.println("-----END DSA PRIVATE KEY-----");
                writer.println();
                writer.println("-----BEGIN DSA PRIVATE KEY (BASE64)-----");
                writer.println(base64Key);
                writer.println("-----END DSA PRIVATE KEY (BASE64)-----");
                
                keyStatusLabel.setText("Đã lưu khóa bí mật vào: " + file.getName());
                keyStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
            } catch (Exception ex) {
                keyStatusLabel.setText("Lỗi lưu khóa bí mật: " + ex.getMessage());
                keyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            }
        }
    }

    /**
     * Lưu khóa công khai ra file
     */
    @FXML
    private void handleSavePublicKey() {
        if (!dsaModel.hasKeyPair()) {
            keyStatusLabel.setText("Chưa có cặp khóa! Vui lòng sinh khóa trước.");
            keyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            return;
        }
        
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Lưu Khóa Công Khai DSA");
        fileChooser.setInitialFileName("dsa_public_key.txt");
        fileChooser.getExtensionFilters().addAll(
            new FileChooser.ExtensionFilter("Text Files (*.txt)", "*.txt"),
            new FileChooser.ExtensionFilter("Key Files (*.key)", "*.key"),
            new FileChooser.ExtensionFilter("All Files (*.*)", "*.*")
        );
        
        Stage stage = (Stage) publicKeyArea.getScene().getWindow();
        File file = fileChooser.showSaveDialog(stage);
        
        if (file != null) {
            try (PrintWriter writer = new PrintWriter(file)) {
                DSAPublicKey pubKey = (DSAPublicKey) dsaModel.getKeyPair().getPublic();
                String base64Key = Base64.getEncoder().encodeToString(pubKey.getEncoded());
                
                writer.println("-----BEGIN DSA PUBLIC KEY-----");
                writer.println("p: " + dsaModel.getP().toString(16).toUpperCase());
                writer.println("q: " + dsaModel.getQ().toString(16).toUpperCase());
                writer.println("g: " + dsaModel.getG().toString(16).toUpperCase());
                writer.println("y: " + dsaModel.getY().toString(16).toUpperCase());
                writer.println("-----END DSA PUBLIC KEY-----");
                writer.println();
                writer.println("-----BEGIN DSA PUBLIC KEY (BASE64)-----");
                writer.println(base64Key);
                writer.println("-----END DSA PUBLIC KEY (BASE64)-----");
                
                keyStatusLabel.setText("Đã lưu khóa công khai vào: " + file.getName());
                keyStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
            } catch (Exception ex) {
                keyStatusLabel.setText("Lỗi lưu khóa công khai: " + ex.getMessage());
                keyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            }
        }
    }

    /**
     * Tải khóa bí mật từ file
     */
    @FXML
    private void handleLoadPrivateKey() {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Tải Khóa Bí Mật DSA");
        fileChooser.getExtensionFilters().addAll(
            new FileChooser.ExtensionFilter("Text/Key Files (*.txt, *.key)", "*.txt", "*.key"),
            new FileChooser.ExtensionFilter("All Files (*.*)", "*.*")
        );
        
        Stage stage = (Stage) messageArea.getScene().getWindow();
        File file = fileChooser.showOpenDialog(stage);
        
        if (file != null) {
            try {
                BigInteger p = null, q = null, g = null, x = null;
                try (java.io.BufferedReader reader = new java.io.BufferedReader(new java.io.FileReader(file))) {
                    String line;
                    while ((line = reader.readLine()) != null) {
                        line = line.trim();
                        if (line.toLowerCase().startsWith("p:")) {
                            p = new BigInteger(line.substring(2).trim(), 16);
                        } else if (line.toLowerCase().startsWith("q:")) {
                            q = new BigInteger(line.substring(2).trim(), 16);
                        } else if (line.toLowerCase().startsWith("g:")) {
                            g = new BigInteger(line.substring(2).trim(), 16);
                        } else if (line.toLowerCase().startsWith("x:")) {
                            x = new BigInteger(line.substring(2).trim(), 16);
                        }
                    }
                }
                
                if (p != null && q != null && g != null && x != null) {
                    dsaModel.setPrivateKeyAndParams(p, q, g, x);
                    signKeyStatusLabel.setText("Đã tải khóa từ file: " + file.getName() + " (" + p.bitLength() + " bit)");
                    signKeyStatusLabel.setStyle("-fx-text-fill: #34d399;");
                    signStatusLabel.setText("Tải khóa bí mật thành công!");
                    signStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
                    
                    // Cập nhật nhãn trạng thái bên tab xác minh vì khóa mới vừa được tải
                    verifyKeyStatusLabel.setText("Đã đồng bộ khóa từ file bí mật (" + p.bitLength() + " bit)");
                    verifyKeyStatusLabel.setStyle("-fx-text-fill: #34d399;");

                    // Đồng bộ giao diện Tab 1
                    paramPArea.setText(p.toString(16).toUpperCase());
                    paramQArea.setText(q.toString(16).toUpperCase());
                    paramGArea.setText(g.toString(16).toUpperCase());
                    publicKeyArea.setText(dsaModel.getY().toString(16).toUpperCase());
                    pBitLengthLabel.setText("Độ dài: " + p.bitLength() + " bit");
                    qBitLengthLabel.setText("Độ dài: " + q.bitLength() + " bit");
                    keyResultBox.setVisible(true);
                    keyResultBox.setManaged(true);
                    keyStatusLabel.setText("Đã đồng bộ khóa bí mật vừa tải");
                    keyStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
                    
                    actualPrivateKeyHex = x.toString(16).toUpperCase();
                    if (isPrivateKeyVisible) {
                        privateKeyArea.setText(actualPrivateKeyHex);
                        if (togglePrivateKeyBtn != null) togglePrivateKeyBtn.setText("Ẩn");
                    } else {
                        privateKeyArea.setText("•".repeat(Math.min(actualPrivateKeyHex.length(), 64)));
                        if (togglePrivateKeyBtn != null) togglePrivateKeyBtn.setText("Hiện");
                    }
                    updateKeyDetailsUI();
                } else {
                    signStatusLabel.setText("Lỗi: Định dạng file khóa bí mật không hợp lệ!");
                    signStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
                }
            } catch (Exception ex) {
                signStatusLabel.setText("Lỗi đọc file khóa: " + ex.getMessage());
                signStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            }
        }
    }

    /**
     * Tải khóa công khai từ file
     */
    @FXML
    private void handleLoadPublicKey() {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Tải Khóa Công Khai DSA");
        fileChooser.getExtensionFilters().addAll(
            new FileChooser.ExtensionFilter("Text/Key Files (*.txt, *.key)", "*.txt", "*.key"),
            new FileChooser.ExtensionFilter("All Files (*.*)", "*.*")
        );
        
        Stage stage = (Stage) verifyMessageArea.getScene().getWindow();
        File file = fileChooser.showOpenDialog(stage);
        
        if (file != null) {
            try {
                BigInteger p = null, q = null, g = null, y = null;
                try (java.io.BufferedReader reader = new java.io.BufferedReader(new java.io.FileReader(file))) {
                    String line;
                    while ((line = reader.readLine()) != null) {
                        line = line.trim();
                        if (line.toLowerCase().startsWith("p:")) {
                            p = new BigInteger(line.substring(2).trim(), 16);
                        } else if (line.toLowerCase().startsWith("q:")) {
                            q = new BigInteger(line.substring(2).trim(), 16);
                        } else if (line.toLowerCase().startsWith("g:")) {
                            g = new BigInteger(line.substring(2).trim(), 16);
                        } else if (line.toLowerCase().startsWith("y:")) {
                            y = new BigInteger(line.substring(2).trim(), 16);
                        }
                    }
                }
                
                if (p != null && q != null && g != null && y != null) {
                    dsaModel.setPublicKeyAndParams(p, q, g, y);
                    verifyKeyStatusLabel.setText("Đã tải khóa từ file: " + file.getName() + " (" + p.bitLength() + " bit)");
                    verifyKeyStatusLabel.setStyle("-fx-text-fill: #34d399;");
                    verifyStatusLabel.setText("Tải khóa công khai thành công!");
                    verifyStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");

                    // Đồng bộ giao diện Tab 1
                    actualPrivateKeyHex = "";
                    paramPArea.setText(p.toString(16).toUpperCase());
                    paramQArea.setText(q.toString(16).toUpperCase());
                    paramGArea.setText(g.toString(16).toUpperCase());
                    privateKeyArea.setText("Không khả dụng (chỉ tải khóa công khai)");
                    publicKeyArea.setText(y.toString(16).toUpperCase());
                    pBitLengthLabel.setText("Độ dài: " + p.bitLength() + " bit");
                    qBitLengthLabel.setText("Độ dài: " + q.bitLength() + " bit");
                    keyResultBox.setVisible(true);
                    keyResultBox.setManaged(true);
                    keyStatusLabel.setText("Đã đồng bộ khóa công khai vừa tải");
                    keyStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
                    if (togglePrivateKeyBtn != null) togglePrivateKeyBtn.setText("Hiện");
                    updateKeyDetailsUI();
                } else {
                    verifyStatusLabel.setText("Lỗi: Định dạng file khóa công khai không hợp lệ!");
                    verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
                }
            } catch (Exception ex) {
                verifyStatusLabel.setText("Lỗi đọc file khóa: " + ex.getMessage());
                verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            }
        }
    }

    /**
     * Đọc nội dung file text mã hóa UTF-8
     */
    private String readTextFile(File file) throws Exception {
        StringBuilder sb = new StringBuilder();
        try (java.io.BufferedReader reader = new java.io.BufferedReader(
                new java.io.InputStreamReader(new java.io.FileInputStream(file), java.nio.charset.StandardCharsets.UTF_8))) {
            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line).append("\n");
            }
        }
        if (sb.length() > 0) {
            sb.setLength(sb.length() - 1);
        }
        return sb.toString();
    }

    /**
     * Tải thông điệp từ file để ký
     */
    @FXML
    private void handleLoadMessageFile() {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Tải Thông Điệp Cần Ký");
        fileChooser.getExtensionFilters().addAll(
            new FileChooser.ExtensionFilter("Text Files (*.txt)", "*.txt"),
            new FileChooser.ExtensionFilter("All Files (*.*)", "*.*")
        );
        
        Stage stage = (Stage) messageArea.getScene().getWindow();
        File file = fileChooser.showOpenDialog(stage);
        
        if (file != null) {
            try {
                String content = readTextFile(file);
                messageArea.setText(content);
                signStatusLabel.setText("Đã tải thông điệp từ file: " + file.getName());
                signStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
            } catch (Exception ex) {
                signStatusLabel.setText("Lỗi đọc file thông điệp: " + ex.getMessage());
                signStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            }
        }
    }

    /**
     * Tải thông điệp để xác minh
     */
    @FXML
    private void handleLoadVerifyMessageFile() {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Tải Thông Điệp Xác Minh");
        fileChooser.getExtensionFilters().addAll(
            new FileChooser.ExtensionFilter("Text Files (*.txt)", "*.txt"),
            new FileChooser.ExtensionFilter("All Files (*.*)", "*.*")
        );
        
        Stage stage = (Stage) verifyMessageArea.getScene().getWindow();
        File file = fileChooser.showOpenDialog(stage);
        
        if (file != null) {
            try {
                String content = readTextFile(file);
                verifyMessageArea.setText(content);
                verifyStatusLabel.setText("Đã tải thông điệp xác minh từ file: " + file.getName());
                verifyStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
            } catch (Exception ex) {
                verifyStatusLabel.setText("Lỗi đọc file thông điệp: " + ex.getMessage());
                verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            }
        }
    }

    /**
     * Tải chữ ký để xác minh
     */
    @FXML
    private void handleLoadVerifySignatureFile() {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Tải Chữ Ký Xác Minh");
        fileChooser.getExtensionFilters().addAll(
            new FileChooser.ExtensionFilter("Signature/Text Files (*.txt, *.sig)", "*.txt", "*.sig"),
            new FileChooser.ExtensionFilter("All Files (*.*)", "*.*")
        );
        
        Stage stage = (Stage) verifySignatureArea.getScene().getWindow();
        File file = fileChooser.showOpenDialog(stage);
        
        if (file != null) {
            try {
                String content = readTextFile(file);
                String base64Sig = content.trim();
                if (content.contains("-----BEGIN DSA SIGNATURE-----")) {
                    int start = content.indexOf("-----BEGIN DSA SIGNATURE-----") + "-----BEGIN DSA SIGNATURE-----".length();
                    int end = content.indexOf("-----END DSA SIGNATURE-----");
                    if (end > start) {
                        base64Sig = content.substring(start, end).trim();
                    }
                }
                verifySignatureArea.setText(base64Sig);
                verifyStatusLabel.setText("Đã tải chữ ký từ file: " + file.getName());
                verifyStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
            } catch (Exception ex) {
                verifyStatusLabel.setText("Lỗi đọc file chữ ký: " + ex.getMessage());
                verifyStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            }
        }
    }

    /**
     * Bật/tắt hiển thị khóa bí mật
     */
    @FXML
    private void handleTogglePrivateKey() {
        if (actualPrivateKeyHex == null || actualPrivateKeyHex.isEmpty()) {
            return;
        }
        
        isPrivateKeyVisible = !isPrivateKeyVisible;
        if (isPrivateKeyVisible) {
            privateKeyArea.setText(actualPrivateKeyHex);
            if (togglePrivateKeyBtn != null) togglePrivateKeyBtn.setText("Ẩn");
        } else {
            privateKeyArea.setText("•".repeat(Math.min(actualPrivateKeyHex.length(), 64)));
            if (togglePrivateKeyBtn != null) togglePrivateKeyBtn.setText("Hiện");
        }
    }

    /**
     * Cập nhật các trường hiển thị thông số chi tiết của khóa dùng để ký/xác minh
     */
    private void updateKeyDetailsUI() {
        BigInteger p = dsaModel.getP();
        BigInteger q = dsaModel.getQ();
        BigInteger g = dsaModel.getG();
        BigInteger x = dsaModel.getX();
        BigInteger y = dsaModel.getY();

        // Cập nhật các trường cho Tab Ký Số
        if (p != null) signParamPArea.setText(p.toString(16).toUpperCase());
        if (q != null) signParamQArea.setText(q.toString(16).toUpperCase());
        if (g != null) signParamGArea.setText(g.toString(16).toUpperCase());
        
        if (x != null) {
            String hexX = x.toString(16).toUpperCase();
            if (isSignPrivateKeyVisible) {
                signPrivateKeyArea.setText(hexX);
                if (toggleSignPrivateKeyBtn != null) toggleSignPrivateKeyBtn.setText("Ẩn");
            } else {
                signPrivateKeyArea.setText("•".repeat(Math.min(hexX.length(), 64)));
                if (toggleSignPrivateKeyBtn != null) toggleSignPrivateKeyBtn.setText("Hiện");
            }
        } else {
            signPrivateKeyArea.setText("Không khả dụng (chỉ tải khóa công khai)");
        }

        // Cập nhật các trường cho Tab Xác Minh
        if (p != null) verifyParamPArea.setText(p.toString(16).toUpperCase());
        if (q != null) verifyParamQArea.setText(q.toString(16).toUpperCase());
        if (g != null) verifyParamGArea.setText(g.toString(16).toUpperCase());
        if (y != null) verifyPublicKeyArea.setText(y.toString(16).toUpperCase());
    }

    /**
     * Ẩn/hiện chi tiết tham số khóa dùng để ký
     */
    @FXML
    private void handleToggleSignKeyDetails() {
        isSignKeyDetailsVisible = !isSignKeyDetailsVisible;
        signKeyDetailsBox.setVisible(isSignKeyDetailsVisible);
        signKeyDetailsBox.setManaged(isSignKeyDetailsVisible);
        toggleSignKeyDetailsBtn.setText(isSignKeyDetailsVisible ? "Thu Gọn" : "Xem Chi Tiết");
        if (isSignKeyDetailsVisible) {
            updateKeyDetailsUI();
        }
    }

    /**
     * Ẩn/hiện chi tiết tham số khóa dùng để xác minh
     */
    @FXML
    private void handleToggleVerifyKeyDetails() {
        isVerifyKeyDetailsVisible = !isVerifyKeyDetailsVisible;
        verifyKeyDetailsBox.setVisible(isVerifyKeyDetailsVisible);
        verifyKeyDetailsBox.setManaged(isVerifyKeyDetailsVisible);
        toggleVerifyKeyDetailsBtn.setText(isVerifyKeyDetailsVisible ? "Thu Gọn" : "Xem Chi Tiết");
        if (isVerifyKeyDetailsVisible) {
            updateKeyDetailsUI();
        }
    }

    /**
     * Bật/tắt hiển thị khóa bí mật trong phần chi tiết ký
     */
    @FXML
    private void handleToggleSignPrivateKey() {
        if (dsaModel.getX() == null) {
            return;
        }
        
        isSignPrivateKeyVisible = !isSignPrivateKeyVisible;
        String hexX = dsaModel.getX().toString(16).toUpperCase();
        if (isSignPrivateKeyVisible) {
            signPrivateKeyArea.setText(hexX);
            if (toggleSignPrivateKeyBtn != null) toggleSignPrivateKeyBtn.setText("Ẩn");
        } else {
            signPrivateKeyArea.setText("•".repeat(Math.min(hexX.length(), 64)));
            if (toggleSignPrivateKeyBtn != null) toggleSignPrivateKeyBtn.setText("Hiện");
        }
    }

    /**
     * Lưu chữ ký số ra file
     */
    @FXML
    private void handleSaveSignature() {
        String signature = signatureArea.getText();
        if (signature == null || signature.trim().isEmpty()) {
            signStatusLabel.setText("Chưa có chữ ký! Vui lòng thực hiện ký số trước.");
            signStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            return;
        }
        
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Lưu Chữ Ký Số DSA");
        fileChooser.setInitialFileName("dsa_signature.sig");
        fileChooser.getExtensionFilters().addAll(
            new FileChooser.ExtensionFilter("Signature Files (*.sig)", "*.sig"),
            new FileChooser.ExtensionFilter("Text Files (*.txt)", "*.txt"),
            new FileChooser.ExtensionFilter("All Files (*.*)", "*.*")
        );
        
        Stage stage = (Stage) signatureArea.getScene().getWindow();
        File file = fileChooser.showSaveDialog(stage);
        
        if (file != null) {
            try (PrintWriter writer = new PrintWriter(file)) {
                writer.println("-----BEGIN DSA SIGNATURE-----");
                writer.println(signature.trim());
                writer.println("-----END DSA SIGNATURE-----");
                
                signStatusLabel.setText("Đã lưu chữ ký vào: " + file.getName());
                signStatusLabel.setStyle("-fx-text-fill: #2dd4a0;");
            } catch (Exception ex) {
                signStatusLabel.setText("Lỗi lưu chữ ký: " + ex.getMessage());
                signStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            }
        }
    }

    /**
     * Bật/tắt hiển thị chi tiết các thành phần r, s của chữ ký
     */
    @FXML
    private void handleToggleSigDetails() {
        String signature = signatureArea.getText();
        if (signature == null || signature.trim().isEmpty()) {
            signStatusLabel.setText("Chưa có chữ ký! Vui lòng thực hiện ký số trước.");
            signStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            return;
        }

        isSigDetailsVisible = !isSigDetailsVisible;
        sigDetailsBox.setVisible(isSigDetailsVisible);
        sigDetailsBox.setManaged(isSigDetailsVisible);
        toggleSigDetailsBtn.setText(isSigDetailsVisible ? "Thu Gọn Chữ Ký" : "Tách Chữ Ký (r, s)");

        if (isSigDetailsVisible) {
            updateSigDetailsUI();
        }
    }

    /**
     * Cập nhật các trường r, s trong VBox chi tiết
     */
    private void updateSigDetailsUI() {
        try {
            byte[] sigBytes = dsaModel.getLastSignatureBytes();
            if (sigBytes == null) {
                String base64Sig = signatureArea.getText().trim();
                if (base64Sig.contains("-----BEGIN DSA SIGNATURE-----")) {
                    int start = base64Sig.indexOf("-----BEGIN DSA SIGNATURE-----") + "-----BEGIN DSA SIGNATURE-----".length();
                    int end = base64Sig.indexOf("-----END DSA SIGNATURE-----");
                    if (end > start) {
                        base64Sig = base64Sig.substring(start, end).trim();
                    }
                }
                sigBytes = Base64.getDecoder().decode(base64Sig);
            }

            BigInteger[] rs = decodeDSASignature(sigBytes);
            BigInteger r = rs[0];
            BigInteger s = rs[1];

            sigRAreaHex.setText(r.toString(16).toUpperCase());
            sigRAreaDec.setText(r.toString(10));
            sigSAreaHex.setText(s.toString(16).toUpperCase());
            sigSAreaDec.setText(s.toString(10));
        } catch (Exception e) {
            signStatusLabel.setText("Lỗi tách chữ ký: " + e.getMessage());
            signStatusLabel.setStyle("-fx-text-fill: #f43f5e;");
            sigDetailsBox.setVisible(false);
            sigDetailsBox.setManaged(false);
            isSigDetailsVisible = false;
            toggleSigDetailsBtn.setText("Tách Chữ Ký (r, s)");
        }
    }

    /**
     * Giải mã chữ ký số dạng DER ASN.1 để lấy r và s
     */
    private BigInteger[] decodeDSASignature(byte[] sigBytes) throws Exception {
        int index = 0;
        if (sigBytes[index++] != 0x30) {
            throw new IllegalArgumentException("Định dạng DER không hợp lệ: Thiếu SEQUENCE tag");
        }
        
        int seqLen = sigBytes[index++] & 0xFF;
        if (seqLen > 127) {
            int lenBytes = seqLen - 128;
            index += lenBytes;
        }
        
        if (sigBytes[index++] != 0x02) {
            throw new IllegalArgumentException("Định dạng DER không hợp lệ: Thiếu INTEGER tag cho r");
        }
        int rLen = sigBytes[index++] & 0xFF;
        byte[] rBytes = new byte[rLen];
        System.arraycopy(sigBytes, index, rBytes, 0, rLen);
        index += rLen;
        BigInteger r = new BigInteger(rBytes);
        
        if (sigBytes[index++] != 0x02) {
            throw new IllegalArgumentException("Định dạng DER không hợp lệ: Thiếu INTEGER tag cho s");
        }
        int sLen = sigBytes[index++] & 0xFF;
        byte[] sBytes = new byte[sLen];
        System.arraycopy(sigBytes, index, sBytes, 0, sLen);
        BigInteger s = new BigInteger(sBytes);
        
        return new BigInteger[]{r, s};
    }
}
