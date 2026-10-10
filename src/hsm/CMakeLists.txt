# ==== HSM library ====
add_library(hsm
    src/HsmManager.cpp
    src/LibtropicSecureElement.cpp
)

target_include_directories(hsm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hsm PRIVATE
    tropic
    audit
    common
    OpenSSL::Crypto
)

# ==== Tests ====
add_executable(test_hsm
    test/HsmManager.test.cpp
)

target_include_directories(test_hsm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/test
)

target_link_libraries(test_hsm PRIVATE
    hsm
    audit
    common
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(test_hsm)