cmake_minimum_required(VERSION 3.21.0)
project(hsmctl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)
include(ExternalProject)

# ==== SQLite3 ====
find_package(SQLite3 REQUIRED)

# ==== OpenSSL ====
find_package(OpenSSL 3.0 REQUIRED)

# ==== Catch2 ====
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)

# ==== PKCS#11 headers (official OASIS v3.0) ====
FetchContent_Declare(
    pkcs11_headers
    GIT_REPOSITORY https://github.com/oasis-tcs/pkcs11.git
    GIT_TAG bcc6fac8759e68f4b0a18adfbf5b169809e29e03
)

FetchContent_MakeAvailable(Catch2 pkcs11_headers)

# ==== SPI/GPIO device paths for TROPIC01 ====
if(NOT DEFINED LT_SPI_DEV_PATH)
    set(LT_SPI_DEV_PATH "/dev/spidev0.0")
endif()
if(NOT DEFINED LT_GPIO_DEV_PATH)
    set(LT_GPIO_DEV_PATH "/dev/gpiochip0")
endif()

# ==== TROPIC01 PKCS#11 module ====
# Built separately: it brings its own libtropic. hsmctl loads it at runtime.
ExternalProject_Add(libtropic_pkcs11
    GIT_REPOSITORY https://github.com/tropicsquare/libtropic-pkcs11.git
    GIT_TAG 604cb444c1deea6a73c4300b36f468083586c6bc
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DLT_HAL=linux_spi
        -DLT_SPI_DEV_PATH=${LT_SPI_DEV_PATH}
        -DLT_GPIO_DEV_PATH=${LT_GPIO_DEV_PATH}
        -DLT_SPI_CS_PIN=25
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS <BINARY_DIR>/libtropic_pkcs11.so
)
ExternalProject_Get_Property(libtropic_pkcs11 BINARY_DIR)
set(TROPIC_PKCS11_MODULE "${BINARY_DIR}/libtropic_pkcs11.so")

# ==== CTest ====
include(CTest)
enable_testing()

# ==== Subdirectories ====
add_subdirectory(src)