cmake_minimum_required(VERSION 3.16)

project(moi LANGUAGES CXX) # Hoặc đặt là project(moi) tùy bạn muốn

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CHÚ Ý: Bật MOC, RCC nhưng TẮT AUTOUIC vì chúng ta code giao diện hoàn toàn bằng tay (C++ UI)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF) # <--- ĐỔI THÀNH OFF

# Tìm Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Tìm OpenSSL
find_package(OpenSSL REQUIRED)

# Tìm và cấu hình thư viện GMP
find_path(GMP_INCLUDE_DIR NAMES gmp.h PATHS "C:/msys64/ucrt64/include")
find_library(GMP_LIBRARIES NAMES gmp PATHS "C:/msys64/ucrt64/lib")
include_directories(${GMP_INCLUDE_DIR})

# Cấu hình file nguồn (Bỏ hoàn toàn UI_FILES)
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/dsa_model.cpp
)

set(HEADERS
    src/mainwindow.h
    src/dsa_model.h
)

# Tạo executable
add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    resources/resources.qrc
)

# Liên kết thư viện
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    OpenSSL::Crypto
    OpenSSL::SSL
    ${GMP_LIBRARIES}
)