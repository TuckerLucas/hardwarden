add_library(common INTERFACE)

target_include_directories(common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ==== Tests ====
add_executable(test_common
    test/Types.test.cpp
)

target_link_libraries(test_common PRIVATE
    Catch2::Catch2WithMain
    common
)

include(CTest)
include(Catch)
catch_discover_tests(test_common)
