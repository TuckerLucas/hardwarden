# ==== CLI executable ====
add_executable(hwarden
    src/main.cpp
    src/CliParser.cpp
    src/CliDisplay.cpp
)

target_include_directories(hwarden PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hwarden 
                      PRIVATE device
                      PRIVATE audit
                      PRIVATE common)

target_compile_definitions(hwarden PRIVATE
    TROPIC_PKCS11_MODULE_PATH="${TROPIC_PKCS11_MODULE}"
)
add_dependencies(hwarden libtropic_pkcs11)

install(TARGETS hwarden DESTINATION /usr/local/bin)

# ==== Tests ====
add_executable(test_cli
    test/CliParser.test.cpp
    src/CliParser.cpp
)

target_include_directories(test_cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_cli PRIVATE
    Catch2::Catch2WithMain
    common
)

include(CTest)
include(Catch)
catch_discover_tests(test_cli)