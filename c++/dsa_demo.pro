QT += core gui widgets

CONFIG += c++17

TARGET   = dsa_demo
TEMPLATE = app

# -------------------------------------------------------
# Source files (UI built programmatically - no .ui file)
# -------------------------------------------------------
SOURCES += \
    src/main.cpp \
    src/dsa_model.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/dsa_model.h \
    src/mainwindow.h

RESOURCES += resources/resources.qrc

# -------------------------------------------------------
# Thư viện GMP + OpenSSL từ MSYS2 (MinGW 64-bit)
# Đường dẫn mặc định khi cài MSYS2 vào C:\msys64
# -------------------------------------------------------
INCLUDEPATH += C:/msys64/mingw64/include

LIBS += -LC:/msys64/mingw64/lib \
        -lgmp \
        -lcrypto \
        -lws2_32 \
        -lcrypt32

# -------------------------------------------------------
# Cài đặt compiler
# -------------------------------------------------------
QMAKE_CXXFLAGS += -std=c++17

# Suppress deprecated OpenSSL warnings
DEFINES += OPENSSL_SUPPRESS_DEPRECATED

# -------------------------------------------------------
# Output directory
# -------------------------------------------------------
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR     = $$PWD/build/moc
RCC_DIR     = $$PWD/build/rcc

# -------------------------------------------------------
# Windows: không hiện console window khi chạy
# -------------------------------------------------------
win32 {
    CONFIG += windows
    RC_ICONS =
}
