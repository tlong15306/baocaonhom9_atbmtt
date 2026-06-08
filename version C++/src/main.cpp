/*
 * main.cpp — Entry point của ứng dụng Qt
 * Nhóm 9 — An Toàn Bảo Mật Thông Tin, 2025-2026
 */

#include "mainwindow.h"
#include <QApplication>
#include <QFont>

int main(int argc, char *argv[]) {
    // Để tránh việc GUI bị mờ trên màn hình High DPI (4K, laptop)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication a(argc, argv);

    // Thiết lập font chữ hệ thống đẹp mắt
    QFont defaultFont("Segoe UI", 9);
    // Nếu chạy trên macOS, dùng SF Pro / System Font
#ifdef Q_OS_MAC
    defaultFont.setFamily(".AppleSystemUIFont");
#endif
    QApplication::setFont(defaultFont);

    MainWindow w;
    w.show();

    return a.exec();
}
