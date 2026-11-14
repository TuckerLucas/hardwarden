cmake_minimum_required(VERSION 3.21.0)
project(hardwarden)

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

# ==== TROPIC01 PKCS#11 modules ====
# The module's connection (HAL) is chosen at compile time, so it is built twice:
# once for SPI (Raspberry Pi shield) and once for USB (USB devkit).
# Each build brings its own libtropic. hardwarden loads the right one at runtime.
set(LIBTROPIC_PKCS11_REPO https://github.com/tropicsquare/libtropic-pkcs11.git)
set(LIBTROPIC_PKCS11_TAG 604cb444c1deea6a73c4300b36f468083586c6bc)

if(NOT DEFINED TS_USB_DEV)
    set(TS_USB_DEV "/dev/ttyACM0")
endif()

ExternalProject_Add(libtropic_pkcs11_spi
    GIT_REPOSITORY ${LIBTROPIC_PKCS11_REPO}
    GIT_TAG ${LIBTROPIC_PKCS11_TAG}
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DLT_HAL=linux_spi
        -DLT_SPI_DEV_PATH=${LT_SPI_DEV_PATH}
        -DLT_GPIO_DEV_PATH=${LT_GPIO_DEV_PATH}
        -DLT_SPI_CS_PIN=25
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS <BINARY_DIR>/libtropic_pkcs11.so
)
ExternalProject_Get_Property(libtropic_pkcs11_spi BINARY_DIR)
set(TROPIC01_SPI_PKCS11_MODULE "${BINARY_DIR}/libtropic_pkcs11.so")

ExternalProject_Add(libtropic_pkcs11_usb
    GIT_REPOSITORY ${LIBTROPIC_PKCS11_REPO}
    GIT_TAG ${LIBTROPIC_PKCS11_TAG}
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DLT_HAL=posix_usb_dongle
        -DTS_USB_DEV=${TS_USB_DEV}
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS <BINARY_DIR>/libtropic_pkcs11.so
)
ExternalProject_Get_Property(libtropic_pkcs11_usb BINARY_DIR)
set(TROPIC01_USB_PKCS11_MODULE "${BINARY_DIR}/libtropic_pkcs11.so")

# ==== CTest ====
include(CTest)
enable_testing()

# ==== Subdirectories ====
add_subdirectory(src)