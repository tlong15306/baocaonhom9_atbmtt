/*
 * mainwindow.cpp — Triển khai giao diện Qt 4 tab
 * Nhóm 9 — An Toàn Bảo Mật Thông Tin, 2025-2026
 */

#include "mainwindow.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QScrollArea>
#include <QSizePolicy>
#include <QFont>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QString>
#include <QDir>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), model_(new DSAModel())
{
    setWindowTitle("DSA Digital Signature — Nhóm 9 ATBMTT");
    setMinimumSize(950, 720);
    resize(1050, 760);

    setupUI();
    loadStyleSheet();
}

MainWindow::~MainWindow() {
    if (worker_) {
        worker_->quit();
        worker_->wait();
    }
    delete model_;
}

void MainWindow::loadStyleSheet() {
    QFile file(":/style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());
    }
}

void MainWindow::setupUI() {
    QWidget* central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupHeader(mainLayout);

    tabWidget_ = new QTabWidget(this);
    tabWidget_->setObjectName("mainTabWidget");
    tabWidget_->setDocumentMode(false);
    tabWidget_->addTab(setupTabKeyGen(), "  🔑  1. Sinh Khóa  ");
    tabWidget_->addTab(setupTabSign(),   "  ✍  2. Ký Số  ");
    tabWidget_->addTab(setupTabVerify(), "  ✅  3. Xác Minh  ");
    tabWidget_->addTab(setupTabGuide(),  "  📖  4. Hướng Dẫn  ");
    mainLayout->addWidget(tabWidget_, 1);

    setupFooter(mainLayout);
}

void MainWindow::setupHeader(QVBoxLayout* mainLayout) {
    QWidget* header = new QWidget();
    header->setObjectName("headerWidget");
    header->setFixedHeight(90);

    QVBoxLayout* hl = new QVBoxLayout(header);
    hl->setContentsMargins(20, 15, 20, 12);
    hl->setSpacing(4);

    QLabel* title = new QLabel("CHỮ KÝ SỐ DSA");
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);

    QLabel* subtitle = new QLabel("Digital Signature Algorithm — Nhóm 9 An Toàn Bảo Mật Thông Tin");
    subtitle->setObjectName("subtitleLabel");
    subtitle->setAlignment(Qt::AlignCenter);

    hl->addWidget(title);
    hl->addWidget(subtitle);
    mainLayout->addWidget(header);
}

void MainWindow::setupFooter(QVBoxLayout* mainLayout) {
    QWidget* footer = new QWidget();
    footer->setObjectName("footerWidget");
    footer->setFixedHeight(36);

    QHBoxLayout* fl = new QHBoxLayout(footer);
    fl->setContentsMargins(20, 0, 20, 0);

    QLabel* lbl = new QLabel("Nhóm 9 — An Toàn Bảo Mật Thông Tin  |  DSA Digital Signature Demo  |  2025-2026");
    lbl->setObjectName("footerLabel");
    lbl->setAlignment(Qt::AlignCenter);
    fl->addWidget(lbl);

    mainLayout->addWidget(footer);
}

QTextEdit* MainWindow::makeReadonlyEdit(int minHeight) {
    QTextEdit* e = new QTextEdit();
    e->setReadOnly(true);
    e->setMinimumHeight(minHeight);
    e->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return e;
}

QLabel* MainWindow::makeSectionTitle(const QString& text) {
    QLabel* l = new QLabel(text);
    l->setObjectName("sectionTitle");
    return l;
}

QLabel* MainWindow::makeFieldLabel(const QString& text) {
    QLabel* l = new QLabel(text);
    l->setObjectName("fieldLabel");
    return l;
}

QPushButton* MainWindow::makeButton(const QString& text, const QString& objName) {
    QPushButton* btn = new QPushButton(text);
    btn->setObjectName(objName);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

void MainWindow::setStatus(QLabel* label, const QString& text, const QString& color) {
    label->setText(text);
    label->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px;").arg(color));
}

QWidget* MainWindow::setupTabKeyGen() {
    QScrollArea* scroll = new QScrollArea();
    scroll->setObjectName("tabScroll");
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* content = new QWidget();
    content->setObjectName("tabContent");
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);

    layout->addWidget(makeSectionTitle("SINH CẶP KHÓA DSA"));
    QLabel* desc = new QLabel("Chọn kích thước khóa và nhấn Sinh Khóa để tạo bộ tham số (p, q, g) và cặp khóa (x, y).");
    desc->setObjectName("sectionDesc");
    desc->setWordWrap(true);
    layout->addWidget(desc);

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setSpacing(12);

    QLabel* sizeLabel = new QLabel("Kích thước khóa:");
    sizeLabel->setObjectName("fieldLabel");

    comboKeySize_ = new QComboBox();
    comboKeySize_->setObjectName("comboKeySize");
    comboKeySize_->addItem("1024 bit");
    comboKeySize_->addItem("2048 bit");
    comboKeySize_->setFixedWidth(160);

    btnGenerate_ = makeButton("▶  Sinh Khóa", "btnPrimary");
    connect(btnGenerate_, &QPushButton::clicked, this, &MainWindow::onGenerateKeys);

    progressBar_ = new QProgressBar();
    progressBar_->setObjectName("keyProgress");
    progressBar_->setRange(0, 0);
    progressBar_->setFixedSize(120, 22);
    progressBar_->setVisible(false);

    topRow->addWidget(sizeLabel);
    topRow->addWidget(comboKeySize_);
    topRow->addWidget(btnGenerate_);
    topRow->addWidget(progressBar_);
    topRow->addStretch();
    layout->addLayout(topRow);

    labelKeyStatus_ = new QLabel("");
    labelKeyStatus_->setObjectName("statusLabel");
    layout->addWidget(labelKeyStatus_);

    keyResultWidget_ = new QWidget();
    keyResultWidget_->setObjectName("resultBox");
    QVBoxLayout* resultLayout = new QVBoxLayout(keyResultWidget_);
    resultLayout->setContentsMargins(16, 16, 16, 16);
    resultLayout->setSpacing(12);

    QLabel* sysTitle = new QLabel("THAM SỐ HỆ THỐNG");
    sysTitle->setObjectName("resultSectionTitle");
    resultLayout->addWidget(sysTitle);

    QHBoxLayout* pRow = new QHBoxLayout();
    QLabel* pLabel = new QLabel("Tham số p  (số nguyên tố lớn):");
    pLabel->setObjectName("paramLabel");
    labelPBits_ = new QLabel("");
    labelPBits_->setObjectName("bitLengthBadge");
    pRow->addWidget(pLabel);
    pRow->addWidget(labelPBits_);
    pRow->addStretch();
    resultLayout->addLayout(pRow);
    editParamP_ = makeReadonlyEdit(55);
    editParamP_->setObjectName("paramAreaBlue");
    resultLayout->addWidget(editParamP_);

    QHBoxLayout* qRow = new QHBoxLayout();
    QLabel* qLabel = new QLabel("Tham số q  (ước nguyên tố của p-1):");
    qLabel->setObjectName("paramLabel");
    labelQBits_ = new QLabel("");
    labelQBits_->setObjectName("bitLengthBadge");
    qRow->addWidget(qLabel);
    qRow->addWidget(labelQBits_);
    qRow->addStretch();
    resultLayout->addLayout(qRow);
    editParamQ_ = makeReadonlyEdit(45);
    editParamQ_->setObjectName("paramAreaBlue");
    resultLayout->addWidget(editParamQ_);

    resultLayout->addWidget(new QLabel("Tham số g  (phần tử sinh bậc q):"));
    editParamG_ = makeReadonlyEdit(55);
    editParamG_->setObjectName("paramAreaBlue");
    resultLayout->addWidget(editParamG_);

    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("separator");
    resultLayout->addWidget(sep);

    QLabel* keyTitle = new QLabel("CẶP KHÓA");
    keyTitle->setObjectName("resultSectionTitle");
    resultLayout->addWidget(keyTitle);

    QLabel* privLabel = new QLabel("🔒  Khóa bí mật x  (Private Key — KHÔNG CHIA SẺ):");
    privLabel->setObjectName("privateKeyLabel");
    resultLayout->addWidget(privLabel);
    editPrivKey_ = makeReadonlyEdit(45);
    editPrivKey_->setObjectName("paramAreaRed");
    resultLayout->addWidget(editPrivKey_);

    QLabel* pubLabel = new QLabel("🌐  Khóa công khai y = g^x mod p  (Public Key):");
    pubLabel->setObjectName("publicKeyLabel");
    resultLayout->addWidget(pubLabel);
    editPubKey_ = makeReadonlyEdit(55);
    editPubKey_->setObjectName("paramAreaGreen");
    resultLayout->addWidget(editPubKey_);

    QHBoxLayout* saveRow = new QHBoxLayout();
    QPushButton* btnSavePub = makeButton("💾  Lưu Khóa Công Khai (.txt)", "btnSecondary");
    QPushButton* btnSavePriv = makeButton("🔒  Lưu Khóa Bí Mật (.key)", "btnDanger");
    connect(btnSavePub,  &QPushButton::clicked, this, &MainWindow::onSavePublicKey);
    connect(btnSavePriv, &QPushButton::clicked, this, &MainWindow::onSavePrivateKey);
    saveRow->addWidget(btnSavePub);
    saveRow->addWidget(btnSavePriv);
    saveRow->addStretch();
    resultLayout->addLayout(saveRow);

    layout->addWidget(keyResultWidget_);
    layout->addStretch();

    keyResultWidget_->setVisible(false);

    scroll->setWidget(content);
    return scroll;
}

QWidget* MainWindow::setupTabSign() {
    QScrollArea* scroll = new QScrollArea();
    scroll->setObjectName("tabScroll");
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* content = new QWidget();
    content->setObjectName("tabContent");
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);

    layout->addWidget(makeSectionTitle("KÝ SỐ THÔNG ĐIỆP"));
    QLabel* desc = new QLabel("Nhập thông điệp cần ký. Hệ thống sẽ băm bằng SHA-256 và ký bằng khóa bí mật DSA để tạo cặp chữ ký (r, s).");
    desc->setObjectName("sectionDesc");
    desc->setWordWrap(true);
    layout->addWidget(desc);

    QHBoxLayout* msgHeaderRow = new QHBoxLayout();
    msgHeaderRow->addWidget(makeFieldLabel("Thông điệp (Message):"));
    QPushButton* btnLoad = makeButton("📂 Tải từ file", "btnSmall");
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::onLoadMessage);
    msgHeaderRow->addStretch();
    msgHeaderRow->addWidget(btnLoad);
    layout->addLayout(msgHeaderRow);

    editMessage_ = new QTextEdit();
    editMessage_->setObjectName("inputArea");
    editMessage_->setPlaceholderText("Nhập thông điệp cần ký tại đây...");
    editMessage_->setMinimumHeight(100);
    layout->addWidget(editMessage_);

    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);
    QPushButton* btnSign      = makeButton("✍  Ký Số",     "btnPrimary");
    QPushButton* btnCopyR     = makeButton("📋 Sao Chép r", "btnSecondary");
    QPushButton* btnCopyS     = makeButton("📋 Sao Chép s", "btnSecondary");
    QPushButton* btnAutoFill  = makeButton("➤ Auto điền Tab 3", "btnSecondary");
    QPushButton* btnClearSign = makeButton("🗑  Xóa",        "btnDanger");
    connect(btnSign,      &QPushButton::clicked, this, &MainWindow::onSignMessage);
    connect(btnCopyR,     &QPushButton::clicked, this, &MainWindow::onCopyR);
    connect(btnCopyS,     &QPushButton::clicked, this, &MainWindow::onCopyS);
    connect(btnAutoFill,  &QPushButton::clicked, this, &MainWindow::onAutoFillVerify);
    connect(btnClearSign, &QPushButton::clicked, this, &MainWindow::onClearSign);
    btnRow->addWidget(btnSign);
    btnRow->addWidget(btnCopyR);
    btnRow->addWidget(btnCopyS);
    btnRow->addWidget(btnAutoFill);
    btnRow->addWidget(btnClearSign);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    labelSignStatus_ = new QLabel("");
    labelSignStatus_->setObjectName("statusLabel");
    layout->addWidget(labelSignStatus_);

    layout->addWidget(makeFieldLabel("Giá trị băm SHA-256  H(m):"));
    editHash_ = makeReadonlyEdit(45);
    editHash_->setObjectName("hashArea");
    layout->addWidget(editHash_);

    layout->addWidget(makeFieldLabel("Chữ ký r  (HEX):"));
    editSigR_ = makeReadonlyEdit(55);
    editSigR_->setObjectName("sigArea");
    layout->addWidget(editSigR_);

    layout->addWidget(makeFieldLabel("Chữ ký s  (HEX):"));
    editSigS_ = makeReadonlyEdit(55);
    editSigS_->setObjectName("sigArea");
    layout->addWidget(editSigS_);

    QHBoxLayout* saveRow = new QHBoxLayout();
    QPushButton* btnSaveSig = makeButton("💾  Lưu chữ ký ra file (.sig)", "btnSecondary");
    connect(btnSaveSig, &QPushButton::clicked, this, &MainWindow::onSaveSignature);
    saveRow->addWidget(btnSaveSig);
    saveRow->addStretch();
    layout->addLayout(saveRow);

    layout->addStretch();
    scroll->setWidget(content);
    return scroll;
}

QWidget* MainWindow::setupTabVerify() {
    QScrollArea* scroll = new QScrollArea();
    scroll->setObjectName("tabScroll");
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* content = new QWidget();
    content->setObjectName("tabContent");
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);

    layout->addWidget(makeSectionTitle("XÁC MINH CHỮ KÝ SỐ"));
    QLabel* desc = new QLabel("Nhập thông điệp gốc và cặp chữ ký (r, s). Hệ thống dùng khóa công khai y để kiểm tra tính hợp lệ.");
    desc->setObjectName("sectionDesc");
    desc->setWordWrap(true);
    layout->addWidget(desc);

    layout->addWidget(makeFieldLabel("Thông điệp cần xác minh:"));
    editVerifyMessage_ = new QTextEdit();
    editVerifyMessage_->setObjectName("inputArea");
    editVerifyMessage_->setPlaceholderText("Nhập thông điệp cần xác minh...");
    editVerifyMessage_->setMinimumHeight(80);
    layout->addWidget(editVerifyMessage_);

    layout->addWidget(makeFieldLabel("Khóa công khai y (Public Key):"));
    editVerifyPubKey_ = new QTextEdit();
    editVerifyPubKey_->setObjectName("inputArea");
    editVerifyPubKey_->setPlaceholderText("Dán khóa công khai y (chuỗi HEX)...");
    editVerifyPubKey_->setMinimumHeight(60);
    layout->addWidget(editVerifyPubKey_);

    layout->addWidget(makeFieldLabel("Chữ ký r  (HEX):"));
    editVerifyR_ = new QTextEdit();
    editVerifyR_->setObjectName("inputArea");
    editVerifyR_->setPlaceholderText("Dán giá trị r (chuỗi HEX)...");
    editVerifyR_->setMinimumHeight(60);
    layout->addWidget(editVerifyR_);

    layout->addWidget(makeFieldLabel("Chữ ký s  (HEX):"));
    editVerifyS_ = new QTextEdit();
    editVerifyS_->setObjectName("inputArea");
    editVerifyS_->setPlaceholderText("Dán giá trị s (chuỗi HEX)...");
    editVerifyS_->setMinimumHeight(60);
    layout->addWidget(editVerifyS_);

    QHBoxLayout* btnRow = new QHBoxLayout();
    QPushButton* btnVerify    = makeButton("✅  Xác Minh",         "btnVerify");
    QPushButton* btnLoadSig   = makeButton("📂  Tải chữ ký từ file", "btnSecondary");
    QPushButton* btnClearVer  = makeButton("🗑  Xóa",               "btnDanger");
    connect(btnVerify,   &QPushButton::clicked, this, &MainWindow::onVerifySignature);
    connect(btnLoadSig,  &QPushButton::clicked, this, &MainWindow::onLoadSignature);
    connect(btnClearVer, &QPushButton::clicked, this, &MainWindow::onClearVerify);
    btnRow->addWidget(btnVerify);
    btnRow->addWidget(btnLoadSig);
    btnRow->addWidget(btnClearVer);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    labelVerifyStatus_ = new QLabel("");
    labelVerifyStatus_->setObjectName("statusLabel");
    layout->addWidget(labelVerifyStatus_);

    verifyResultWidget_ = new QWidget();
    verifyResultWidget_->setObjectName("verifyResultBox");
    QVBoxLayout* vrLayout = new QVBoxLayout(verifyResultWidget_);
    vrLayout->setContentsMargins(16, 16, 16, 16);
    vrLayout->setSpacing(10);

    QLabel* midTitle = new QLabel("GIÁ TRỊ TRUNG GIAN");
    midTitle->setObjectName("resultSectionTitle");
    vrLayout->addWidget(midTitle);

    auto makeIntermRow = [&](const QString& labelText, QLabel*& outLabel) {
        QHBoxLayout* row = new QHBoxLayout();
        QLabel* name = new QLabel(labelText);
        name->setObjectName("intermediateLabel");
        name->setFixedWidth(280);
        outLabel = new QLabel("—");
        outLabel->setObjectName("intermediateValue");
        outLabel->setWordWrap(true);
        outLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        row->addWidget(name);
        row->addWidget(outLabel, 1);
        vrLayout->addLayout(row);
    };

    makeIntermRow("H(m) = SHA256(message):", labelVHash_);
    makeIntermRow("w  = s⁻¹ mod q:",         labelVW_);
    makeIntermRow("u₁ = H(m)·w mod q:",      labelVU1_);
    makeIntermRow("u₂ = r·w mod q:",          labelVU2_);
    makeIntermRow("v  = (g^u₁·y^u₂ mod p) mod q:", labelVV_);

    QFrame* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setObjectName("separator");
    vrLayout->addWidget(sep2);

    QHBoxLayout* conclusionRow = new QHBoxLayout();
    conclusionRow->setAlignment(Qt::AlignCenter);
    conclusionRow->setSpacing(16);

    verifyCircle_ = new QFrame();
    verifyCircle_->setObjectName("verifyCircleNeutral");
    verifyCircle_->setFixedSize(56, 56);

    labelVerifyResult_ = new QLabel("KẾT QUẢ XÁC MINH");
    labelVerifyResult_->setObjectName("verifyResultLabel");
    labelVerifyResult_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    conclusionRow->addStretch();
    conclusionRow->addWidget(verifyCircle_);
    conclusionRow->addWidget(labelVerifyResult_);
    conclusionRow->addStretch();
    vrLayout->addLayout(conclusionRow);

    layout->addWidget(verifyResultWidget_);
    layout->addStretch();

    verifyResultWidget_->setVisible(false);

    scroll->setWidget(content);
    return scroll;
}

QWidget* MainWindow::setupTabGuide() {
    QScrollArea* scroll = new QScrollArea();
    scroll->setObjectName("tabScroll");
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* content = new QWidget();
    content->setObjectName("tabContent");
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);

    layout->addWidget(makeSectionTitle("HƯỚNG DẪN SỬ DỤNG"));

    auto makeCard = [&](const QString& title, const QString& body, const QString& cardStyle = "guideCard") {
        QWidget* card = new QWidget();
        card->setObjectName(cardStyle);
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setContentsMargins(16, 14, 16, 14);
        cl->setSpacing(8);

        QLabel* t = new QLabel(title);
        t->setObjectName("guideCardTitle");
        cl->addWidget(t);

        QLabel* b = new QLabel(body);
        b->setObjectName("guideCardText");
        b->setWordWrap(true);
        cl->addWidget(b);

        layout->addWidget(card);
    };

    makeCard("DSA là gì?",
             "DSA (Digital Signature Algorithm) là thuật toán chữ ký số được NIST công bố năm 1991 (FIPS 186). "
             "DSA dùng để xác thực danh tính người gửi và đảm bảo tính toàn vẹn của thông điệp. "
             "Chữ ký số hoạt động dựa trên cặp khóa bất đối xứng: khóa bí mật x (dùng để ký) "
             "và khóa công khai y (dùng để xác minh). Nền tảng toán học: bài toán Logarit rời rạc.");

    makeCard("Bước 1 — Sinh Khóa  🔑",
             "Chuyển sang tab \"1. Sinh Khóa\", chọn kích thước khóa (1024 hoặc 2048 bit), "
             "sau đó nhấn nút Sinh Khóa. Hệ thống sẽ tạo bộ tham số (p, q, g) và cặp khóa "
             "(x — bí mật, y — công khai). Bạn có thể lưu khóa ra file để tái sử dụng.\n"
             "Khóa 2048 bit bảo mật cao hơn nhưng sinh lâu hơn (vài giây).");

    makeCard("Bước 2 — Ký Số  ✍",
             "Chuyển sang tab \"2. Ký Số\", nhập thông điệp hoặc tải từ file, rồi nhấn Ký Số. "
             "Thông điệp được băm bằng SHA-256, sau đó ký bằng khóa bí mật x để tạo ra "
             "cặp chữ ký số (r, s) dạng HEX. Bạn có thể lưu chữ ký ra file .sig "
             "hoặc nhấn \"Auto điền Tab 3\" để xác minh ngay.");

    makeCard("Bước 3 — Xác Minh  ✅",
             "Chuyển sang tab \"3. Xác Minh\", nhập thông điệp gốc và cặp (r, s), "
             "hoặc tải từ file .sig. Nhấn Xác Minh — hệ thống dùng khóa công khai y "
             "để tính toán và so sánh. Kết quả hiển thị đầy đủ các giá trị trung gian "
             "(w, u₁, u₂, v) để kiểm chứng từng bước.");

    makeCard("⚠️  Lưu ý bảo mật",
             "• Phải sinh khóa trước khi ký số hoặc xác minh.\n"
             "• Chữ ký chỉ hợp lệ với đúng thông điệp gốc — thay đổi dù một ký tự cũng khiến xác minh thất bại.\n"
             "• Mỗi lần sinh khóa mới, chữ ký cũ sẽ không còn xác minh được với khóa mới.\n"
             "• Số k được sinh ngẫu nhiên mới sau mỗi lần ký — KHÔNG bao giờ dùng lại k!\n"
             "• Ứng dụng này dùng để minh họa — không dùng cho mục đích bảo mật thực tế.",
             "guideCardNote");

    makeCard("Thông tin Nhóm  👥",
             "Nhóm 9 — Môn An Toàn Bảo Mật Thông Tin\n"
             "Thư viện: GMP (GNU Multiple Precision) + OpenSSL (SHA-256) + Qt 5.15 (GUI)\n"
             "Năm học: 2025-2026");

    layout->addStretch();
    scroll->setWidget(content);
    return scroll;
}

void MainWindow::onGenerateKeys() {
    if (isGenerating_) return;

    QString sizeStr = comboKeySize_->currentText();
    int keyBits = sizeStr.contains("1024") ? 1024 : 2048;

    isGenerating_ = true;
    btnGenerate_->setEnabled(false);
    progressBar_->setVisible(true);
    keyResultWidget_->setVisible(false);
    setStatus(labelKeyStatus_,
              QString("⏳ Đang sinh khóa DSA %1 bit... (vui lòng chờ)").arg(keyBits),
              "#F0AD4E");

    worker_ = new KeyGenWorker(model_, keyBits, this);
    connect(worker_, &KeyGenWorker::finished, this, &MainWindow::onKeyGenFinished);
    connect(worker_, &KeyGenWorker::finished, worker_, &QObject::deleteLater);
    worker_->start();
}

void MainWindow::onKeyGenFinished(bool success) {
    isGenerating_ = false;
    btnGenerate_->setEnabled(true);
    progressBar_->setVisible(false);

    if (!success) {
        setStatus(labelKeyStatus_, "❌ Lỗi: Không thể sinh khóa! Vui lòng thử lại.", "#D9534F");
        return;
    }

    editParamP_->setPlainText(QString::fromStdString(model_->getP_hex()));
    editParamQ_->setPlainText(QString::fromStdString(model_->getQ_hex()));
    editParamG_->setPlainText(QString::fromStdString(model_->getG_hex()));
    editPrivKey_->setPlainText(QString::fromStdString(model_->getX_hex()));
    editPubKey_->setPlainText(QString::fromStdString(model_->getY_hex()));

    labelPBits_->setText(QString("Độ dài: %1 bit").arg(model_->getPBitLength()));
    labelQBits_->setText(QString("Độ dài: %1 bit").arg(model_->getQBitLength()));

    setStatus(labelKeyStatus_,
              QString("✅ Sinh khóa thành công! Kích thước: %1 bit").arg(model_->getKeyBits()),
              "#5CB85C");

    keyResultWidget_->setVisible(true);
    QGraphicsOpacityEffect* fx = new QGraphicsOpacityEffect(keyResultWidget_);
    keyResultWidget_->setGraphicsEffect(fx);
    QPropertyAnimation* anim = new QPropertyAnimation(fx, "opacity");
    anim->setDuration(400);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onSavePublicKey() {
    if (!model_->hasKeyPair()) {
        QMessageBox::warning(this, "Chưa có khóa", "Vui lòng sinh khóa trước!");
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, "Lưu Khóa Công Khai",
                                                QDir::homePath() + "/public_key.txt",
                                                "Text Files (*.txt);;All Files (*)");
    if (path.isEmpty()) return;

    if (model_->savePublicKey(path.toStdString())) {
        setStatus(labelKeyStatus_, "✅ Đã lưu khóa công khai: " + path, "#5CB85C");
    } else {
        QMessageBox::critical(this, "Lỗi", "Không thể lưu file: " + path);
    }
}

void MainWindow::onSavePrivateKey() {
    if (!model_->hasKeyPair()) {
        QMessageBox::warning(this, "Chưa có khóa", "Vui lòng sinh khóa trước!");
        return;
    }
    QMessageBox::warning(this, "⚠️ Cảnh báo Bảo Mật",
                         "Khóa bí mật x sẽ được lưu vào file!\n"
                         "Hãy bảo vệ file này cẩn thận và KHÔNG chia sẻ cho bất kỳ ai.");

    QString path = QFileDialog::getSaveFileName(this, "Lưu Khóa Bí Mật",
                                                QDir::homePath() + "/private_key.key",
                                                "Key Files (*.key);;All Files (*)");
    if (path.isEmpty()) return;

    if (model_->savePrivateKey(path.toStdString())) {
        setStatus(labelKeyStatus_, "🔒 Đã lưu khóa bí mật: " + path, "#5BC0DE");
    } else {
        QMessageBox::critical(this, "Lỗi", "Không thể lưu file: " + path);
    }
}

void MainWindow::onSignMessage() {
    QString message = editMessage_->toPlainText().trimmed();

    if (message.isEmpty()) {
        setStatus(labelSignStatus_, "⚠️ Vui lòng nhập thông điệp cần ký!", "#F0AD4E");
        return;
    }
    if (!model_->hasKeyPair()) {
        setStatus(labelSignStatus_, "⚠️ Chưa có cặp khóa! Vui lòng sinh khóa ở Tab 1.", "#F0AD4E");
        tabWidget_->setCurrentIndex(0);
        return;
    }

    std::string r_out, s_out;
    if (!model_->signMessage(message.toStdString(), r_out, s_out)) {
        setStatus(labelSignStatus_, "❌ Lỗi ký số! Vui lòng thử lại.", "#D9534F");
        return;
    }

    currentR_ = QString::fromStdString(r_out);
    currentS_ = QString::fromStdString(s_out);

    editHash_->setPlainText(QString::fromStdString(model_->getLastHash_hex()));
    editSigR_->setPlainText(currentR_);
    editSigS_->setPlainText(currentS_);

    setStatus(labelSignStatus_, "✅ Thông điệp đã được ký thành công bằng SHA256withDSA!", "#5CB85C");
}

void MainWindow::onClearSign() {
    editMessage_->clear();
    editHash_->clear();
    editSigR_->clear();
    editSigS_->clear();
    currentR_.clear();
    currentS_.clear();
    labelSignStatus_->clear();
}

void MainWindow::onCopyR() {
    if (currentR_.isEmpty()) {
        setStatus(labelSignStatus_, "⚠️ Chưa có chữ ký r để sao chép!", "#F0AD4E");
        return;
    }
    qApp->clipboard()->setText(currentR_);
    setStatus(labelSignStatus_, "📋 Đã sao chép r vào clipboard!", "#5BC0DE");
}

void MainWindow::onCopyS() {
    if (currentS_.isEmpty()) {
        setStatus(labelSignStatus_, "⚠️ Chưa có chữ ký s để sao chép!", "#F0AD4E");
        return;
    }
    qApp->clipboard()->setText(currentS_);
    setStatus(labelSignStatus_, "📋 Đã sao chép s vào clipboard!", "#5BC0DE");
}

void MainWindow::onSaveSignature() {
    if (currentR_.isEmpty() || currentS_.isEmpty()) {
        QMessageBox::warning(this, "Chưa có chữ ký", "Vui lòng ký số trước!");
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, "Lưu Chữ Ký",
                                                QDir::homePath() + "/signature.sig",
                                                "Signature Files (*.sig);;All Files (*)");
    if (path.isEmpty()) return;

    if (model_->saveSignature(path.toStdString(), currentR_.toStdString(), currentS_.toStdString())) {
        setStatus(labelSignStatus_, "💾 Đã lưu chữ ký: " + path, "#5CB85C");
    } else {
        QMessageBox::critical(this, "Lỗi", "Không thể lưu file: " + path);
    }
}

void MainWindow::onLoadMessage() {
    QString path = QFileDialog::getOpenFileName(this, "Tải Thông Điệp",
                                                QDir::homePath(),
                                                "Text Files (*.txt);;All Files (*)");
    if (path.isEmpty()) return;

    QFile f(path);
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&f);
        editMessage_->setPlainText(in.readAll());
    }
}

void MainWindow::onAutoFillVerify() {
    if (currentR_.isEmpty() || currentS_.isEmpty()) {
        setStatus(labelSignStatus_, "⚠️ Chưa có chữ ký! Ký số trước đã.", "#F0AD4E");
        return;
    }
    editVerifyMessage_->setPlainText(editMessage_->toPlainText());
    editVerifyR_->setPlainText(currentR_);
    editVerifyS_->setPlainText(currentS_);

    // Auto điền luôn cả Khóa y
    editVerifyPubKey_->setPlainText(QString::fromStdString(model_->getY_hex()));

    tabWidget_->setCurrentIndex(2);
}

void MainWindow::onVerifySignature() {
    QString message = editVerifyMessage_->toPlainText();
    QString yHex    = editVerifyPubKey_->toPlainText().trimmed();
    QString rHex    = editVerifyR_->toPlainText().trimmed();
    QString sHex    = editVerifyS_->toPlainText().trimmed();

    if (message.isEmpty() || yHex.isEmpty() || rHex.isEmpty() || sHex.isEmpty()) {
        setStatus(labelVerifyStatus_, "⚠️ Vui lòng nhập đầy đủ: Thông điệp, Khóa y, chữ ký r và s!", "#F0AD4E");
        return;
    }
    if (!model_->hasKeyPair()) {
        setStatus(labelVerifyStatus_, "⚠️ Chưa có tham số hệ thống! Vui lòng sinh khóa ở Tab 1.", "#F0AD4E");
        return;
    }

    VerifyDetails details;
    bool valid = model_->verifySignature(message.toStdString(),
                                         rHex.toStdString(),
                                         sHex.toStdString(),
                                         yHex.toStdString(),
                                         details);

    auto shortHex = [](const std::string& s) -> QString {
        QString q = QString::fromStdString(s);
        return q.length() > 48 ? q.left(48) + "..." : q;
    };

    labelVHash_->setText(shortHex(details.hash_hex));
    labelVW_->setText(shortHex(details.w_hex));
    labelVU1_->setText(shortHex(details.u1_hex));
    labelVU2_->setText(shortHex(details.u2_hex));
    labelVV_->setText(shortHex(details.v_hex));

    verifyResultWidget_->setVisible(true);

    if (valid) {
        labelVerifyResult_->setText("✔  CHỮ KÝ HỢP LỆ  (VALID SIGNATURE)");
        labelVerifyResult_->setStyleSheet("color: #5CB85C; font-weight: bold; font-size: 15px;");
        verifyCircle_->setObjectName("verifyCircleValid");
        verifyCircle_->style()->unpolish(verifyCircle_);
        verifyCircle_->style()->polish(verifyCircle_);
        setStatus(labelVerifyStatus_, "✅ Xác minh thành công! Văn bản toàn vẹn và đúng nguồn gốc.", "#5CB85C");
    } else {
        labelVerifyResult_->setText("✘  CHỮ KÝ KHÔNG HỢP LỆ  (INVALID SIGNATURE)");
        labelVerifyResult_->setStyleSheet("color: #D9534F; font-weight: bold; font-size: 15px;");
        verifyCircle_->setObjectName("verifyCircleInvalid");
        verifyCircle_->style()->unpolish(verifyCircle_);
        verifyCircle_->style()->polish(verifyCircle_);

        QString originalMsg = editMessage_->toPlainText();
        QString originalR   = currentR_;
        QString originalS   = currentS_;
        QString originalY   = QString::fromStdString(model_->getY_hex());

        QStringList errors;

        if (!originalR.isEmpty() && !originalY.isEmpty()) {
            if (message != originalMsg) {
                errors << "1. Văn bản không toàn vẹn (thông điệp đã bị sửa đổi).";
            }
            if (rHex != originalR || sHex != originalS) {
                errors << "2. Bản mã (chữ ký r, s) đã bị thay đổi hoặc sai định dạng.";
            }
            if (yHex != originalY) {
                errors << "3. Khóa công khai y đã bị sửa (xác minh sai khóa).";
            }
        }

        if (errors.isEmpty()) {
            setStatus(labelVerifyStatus_, "⚠️ Cảnh báo: Chữ ký, Khóa hoặc Thông điệp không khớp!", "#D9534F");
        } else {
            QString errorMsg = "⚠️ PHÁT HIỆN GIAN LẬN THAY ĐỔI DỮ LIỆU:\n" + errors.join("\n");
            setStatus(labelVerifyStatus_, errorMsg, "#D9534F");
        }
    }

    QGraphicsOpacityEffect* fx = new QGraphicsOpacityEffect(verifyResultWidget_);
    verifyResultWidget_->setGraphicsEffect(fx);
    QPropertyAnimation* anim = new QPropertyAnimation(fx, "opacity");
    anim->setDuration(350);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onClearVerify() {
    editVerifyMessage_->clear();
    editVerifyPubKey_->clear();
    editVerifyR_->clear();
    editVerifyS_->clear();
    labelVerifyStatus_->clear();
    labelVHash_->setText("—");
    labelVW_->setText("—");
    labelVU1_->setText("—");
    labelVU2_->setText("—");
    labelVV_->setText("—");
    verifyResultWidget_->setVisible(false);
}

void MainWindow::onLoadSignature() {
    QString path = QFileDialog::getOpenFileName(this, "Tải Chữ Ký",
                                                QDir::homePath(),
                                                "Signature Files (*.sig);;All Files (*)");
    if (path.isEmpty()) return;

    std::string r_out, s_out;
    if (model_->loadSignature(path.toStdString(), r_out, s_out)) {
        editVerifyR_->setPlainText(QString::fromStdString(r_out));
        editVerifyS_->setPlainText(QString::fromStdString(s_out));
        setStatus(labelVerifyStatus_, "📂 Đã tải chữ ký từ: " + path, "#5BC0DE");
    } else {
        QMessageBox::critical(this, "Lỗi", "Không thể đọc file chữ ký: " + path);
    }
}