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

import java.math.BigInteger;
import java.net.URL;
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

    // === FXML UI Components ===
    
    // Tab Sinh Khóa
    @FXML private ComboBox<String> keySizeCombo;
    @FXML private TextArea paramPArea;
    @FXML private TextArea paramQArea;
    @FXML private TextArea paramGArea;
    @FXML private TextArea privateKeyArea;
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
    
    // Tab Xác Minh
    @FXML private TextArea verifyMessageArea;
    @FXML private TextArea verifySignatureArea;
    @FXML private Label verifyStatusLabel;
    @FXML private VBox verifyResultBox;
    @FXML private Label verifyResultLabel;
    @FXML private Circle verifyIndicator;
    @FXML private Button verifyButton;
    
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
        footerLabel.setText("Nhóm 9 - An Toàn Bảo Mật Thông Tin | DSA Digital Signature Demo | 2025");
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
        keyStatusLabel.setStyle("-fx-text-fill: #f0ad4e;");
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
            keyStatusLabel.setStyle("-fx-text-fill: #5cb85c;");
            
            // Hiển thị các tham số
            BigInteger p = dsaModel.getP();
            BigInteger q = dsaModel.getQ();
            BigInteger g = dsaModel.getG();
            
            paramPArea.setText(p.toString(16).toUpperCase());
            paramQArea.setText(q.toString(16).toUpperCase());
            paramGArea.setText(g.toString(16).toUpperCase());
            privateKeyArea.setText(dsaModel.getX().toString(16).toUpperCase());
            publicKeyArea.setText(dsaModel.getY().toString(16).toUpperCase());
            
            pBitLengthLabel.setText("Độ dài: " + p.bitLength() + " bit");
            qBitLengthLabel.setText("Độ dài: " + q.bitLength() + " bit");
            
            // Hiện kết quả với animation
            keyResultBox.setVisible(true);
            keyResultBox.setManaged(true);
            keyResultBox.setOpacity(0);
            FadeTransition fade = new FadeTransition(Duration.millis(500), keyResultBox);
            fade.setFromValue(0);
            fade.setToValue(1);
            fade.play();
        });
        
        task.setOnFailed(e -> {
            keyProgress.setVisible(false);
            keyStatusLabel.setText("Lỗi sinh khóa: " + task.getException().getMessage());
            keyStatusLabel.setStyle("-fx-text-fill: #d9534f;");
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
                signStatusLabel.setStyle("-fx-text-fill: #f0ad4e;");
                return;
            }
            
            if (!dsaModel.hasKeyPair()) {
                signStatusLabel.setText("Chưa có cặp khóa! Vui lòng sinh khóa trước (Tab 1).");
                signStatusLabel.setStyle("-fx-text-fill: #f0ad4e;");
                
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
            signStatusLabel.setStyle("-fx-text-fill: #5cb85c;");
            
            // Tự động điền vào tab Xác minh
            verifyMessageArea.setText(message);
            verifySignatureArea.setText(signatureBase64);
            
            // Animation nhấp nháy kết quả
            FadeTransition fade = new FadeTransition(Duration.millis(300), signatureArea);
            fade.setFromValue(0.3);
            fade.setToValue(1);
            fade.setCycleCount(2);
            fade.play();
            
        } catch (Exception e) {
            signStatusLabel.setText("Lỗi ký số: " + e.getMessage());
            signStatusLabel.setStyle("-fx-text-fill: #d9534f;");
        }
    }

    /**
     * Xử lý sự kiện nhấn nút "Xác Minh"
     * Kiểm tra tính hợp lệ của chữ ký dựa trên khóa công khai y
     */
    @FXML
    private void handleVerifyAction() {
        try {
            String message = verifyMessageArea.getText();
            String signature = verifySignatureArea.getText();
            
            if (message == null || message.trim().isEmpty()) {
                verifyStatusLabel.setText("Vui lòng nhập thông điệp cần xác minh!");
                verifyStatusLabel.setStyle("-fx-text-fill: #f0ad4e;");
                return;
            }
            
            if (signature == null || signature.trim().isEmpty()) {
                verifyStatusLabel.setText("Vui lòng nhập chữ ký số cần xác minh!");
                verifyStatusLabel.setStyle("-fx-text-fill: #f0ad4e;");
                return;
            }
            
            if (!dsaModel.hasKeyPair()) {
                verifyStatusLabel.setText("Chưa có cặp khóa! Vui lòng sinh khóa trước.");
                verifyStatusLabel.setStyle("-fx-text-fill: #f0ad4e;");
                return;
            }
            
            boolean isValid = dsaModel.verifySignature(message, signature.trim());
            
            verifyResultBox.setVisible(true);
            verifyResultBox.setManaged(true);
            
            if (isValid) {
                verifyResultLabel.setText("CHỮ KÝ HỢP LỆ");
                verifyResultLabel.setStyle("-fx-text-fill: #5cb85c; -fx-font-weight: bold; -fx-font-size: 16px;");
                verifyIndicator.setFill(Color.web("#5cb85c"));
                verifyStatusLabel.setText("Xác minh thành công! Thông điệp chưa bị thay đổi.");
                verifyStatusLabel.setStyle("-fx-text-fill: #5cb85c;");
            } else {
                verifyResultLabel.setText("CHỮ KÝ KHÔNG HỢP LỆ");
                verifyResultLabel.setStyle("-fx-text-fill: #d9534f; -fx-font-weight: bold; -fx-font-size: 16px;");
                verifyIndicator.setFill(Color.web("#d9534f"));
                verifyStatusLabel.setText("Cảnh báo: Thông điệp có thể đã bị giả mạo hoặc chữ ký không đúng!");
                verifyStatusLabel.setStyle("-fx-text-fill: #d9534f;");
            }
            
            // Animation
            ScaleTransition scale = new ScaleTransition(Duration.millis(300), verifyResultBox);
            scale.setFromX(0.8);
            scale.setFromY(0.8);
            scale.setToX(1);
            scale.setToY(1);
            scale.play();
            
        } catch (IllegalArgumentException e) {
            verifyStatusLabel.setText("Chữ ký không đúng định dạng Base64!");
            verifyStatusLabel.setStyle("-fx-text-fill: #d9534f;");
        } catch (Exception e) {
            verifyStatusLabel.setText("Lỗi xác minh: " + e.getMessage());
            verifyStatusLabel.setStyle("-fx-text-fill: #d9534f;");
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
            signStatusLabel.setStyle("-fx-text-fill: #5bc0de;");
        }
    }
}
