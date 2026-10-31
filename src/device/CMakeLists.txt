# ==== Device library ====
add_library(device
    src/DeviceManager.cpp
    src/Pkcs11Device.cpp
)

target_include_directories(device PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${pkcs11_headers_SOURCE_DIR}/published/3-00
)

target_link_libraries(device PRIVATE
    audit
    common
    OpenSSL::Crypto
    ${CMAKE_DL_LIBS}
)

# ==== Tests ====
add_executable(test_device
    test/DeviceManager.test.cpp
)

target_include_directories(test_device PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/test
)

target_link_libraries(test_device PRIVATE
    device
    audit
    common
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(test_device)