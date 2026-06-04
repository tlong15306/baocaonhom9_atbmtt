#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
 * mainwindow.h — Qt MainWindow với giao diện 4 tab
 * Tương đương DSAController.java + FXML trong project Java
 *
 * Nhóm 9 — An Toàn Bảo Mật Thông Tin, 2025-2026
 */

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QTabWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QGroupBox>

#include "dsa_model.h"

// ============================================================
// Worker thread — Sinh khóa trên luồng nền
// Tương đương javafx.concurrent.Task<Void>
// ============================================================
class KeyGenWorker : public QThread {
    Q_OBJECT
public:
    explicit KeyGenWorker(DSAModel* model, int keyBits, QObject* parent = nullptr)
        : QThread(parent), model_(model), keyBits_(keyBits) {}

    void run() override {
        bool ok = model_->generateKeyPair(keyBits_);
        emit finished(ok);
    }

signals:
    void finished(bool success);

private:
    DSAModel* model_;
    int keyBits_;
};

// ============================================================
// MainWindow — Cửa sổ chính với 4 tab
// ============================================================
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // Tab 1: Sinh Khóa
    void onGenerateKeys();
    void onKeyGenFinished(bool success);
    void onSavePublicKey();
    void onSavePrivateKey();

    // Tab 2: Ký Số
    void onSignMessage();
    void onClearSign();
    void onCopyR();
    void onCopyS();
    void onSaveSignature();
    void onLoadMessage();
    void onAutoFillVerify();

    // Tab 3: Xác Minh
    void onVerifySignature();
    void onClearVerify();
    void onLoadSignature();

private:
    // === Setup UI ===
    void setupUI();
    void setupHeader(QVBoxLayout* mainLayout);
    QWidget* setupTabKeyGen();
    QWidget* setupTabSign();
    QWidget* setupTabVerify();
    QWidget* setupTabGuide();
    void setupFooter(QVBoxLayout* mainLayout);
    void loadStyleSheet();

    // === Helpers ===
    void setStatus(QLabel* label, const QString& text, const QString& color);
    QTextEdit* makeReadonlyEdit(int minHeight = 60);
    QLabel* makeSectionTitle(const QString& text);
    QLabel* makeFieldLabel(const QString& text);
    QPushButton* makeButton(const QString& text, const QString& styleClass);

    // === DSA Model ===
    DSAModel* model_;
    KeyGenWorker* worker_ = nullptr;
    bool isGenerating_ = false;

    // Lưu r, s của lần ký gần nhất
    QString currentR_, currentS_;

    // === Widgets — Tab 1: Sinh Khóa ===
    QComboBox*    comboKeySize_;
    QPushButton*  btnGenerate_;
    QProgressBar* progressBar_;
    QLabel*       labelKeyStatus_;
    QWidget*      keyResultWidget_;   // Ẩn/hiện sau khi sinh khóa
    QTextEdit*    editParamP_;
    QTextEdit*    editParamQ_;
    QTextEdit*    editParamG_;
    QTextEdit*    editPrivKey_;
    QTextEdit*    editPubKey_;
    QLabel*       labelPBits_;
    QLabel*       labelQBits_;

    // === Widgets — Tab 2: Ký Số ===
    QTextEdit*   editMessage_;
    QLabel*      labelSignStatus_;
    QTextEdit*   editHash_;
    QTextEdit*   editSigR_;
    QTextEdit*   editSigS_;

    // === Widgets — Tab 3: Xác Minh ===
    QTextEdit*   editVerifyMessage_;
    QTextEdit*   editVerifyR_;
    QTextEdit*   editVerifyS_;
    QLabel*      labelVerifyStatus_;
    QWidget*     verifyResultWidget_;  // Ẩn/hiện sau khi xác minh
    QLabel*      labelVHash_;
    QLabel*      labelVW_;
    QLabel*      labelVU1_;
    QLabel*      labelVU2_;
    QLabel*      labelVV_;
    QFrame*      verifyCircle_;
    QLabel*      labelVerifyResult_;

    // === Tab widget ===
    QTabWidget* tabWidget_;
};

#endif // MAINWINDOW_H
