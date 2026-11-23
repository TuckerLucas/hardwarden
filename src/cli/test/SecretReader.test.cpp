#include "SecretReader.hpp"

#include <catch2/catch_test_macros.hpp>
#include <sstream>

TEST_CASE("readSecret")
{
    std::ostringstream prompt_out;

    SECTION("reads one line")
    {
        std::istringstream in("123456\n");

        REQUIRE(readSecret("Enter PIN: ", in, prompt_out, false) == "123456");
    }

    SECTION("reads only the first line")
    {
        std::istringstream in("123456\nsomething else\n");

        REQUIRE(readSecret("Enter PIN: ", in, prompt_out, false) == "123456");
    }

    SECTION("accepts a missing final newline")
    {
        std::istringstream in("123456");

        REQUIRE(readSecret("Enter PIN: ", in, prompt_out, false) == "123456");
    }

    SECTION("strips a Windows line ending")
    {
        std::istringstream in("123456\r\n");

        REQUIRE(readSecret("Enter PIN: ", in, prompt_out, false) == "123456");
    }

    SECTION("empty line is treated as cancelled")
    {
        std::istringstream in("\n");

        REQUIRE_FALSE(readSecret("Enter PIN: ", in, prompt_out, false).has_value());
    }

    SECTION("no input is treated as cancelled")
    {
        std::istringstream in("");

        REQUIRE_FALSE(readSecret("Enter PIN: ", in, prompt_out, false).has_value());
    }

    SECTION("no prompt is shown when the input is not a terminal")
    {
        std::istringstream in("123456\n");

        readSecret("Enter PIN: ", in, prompt_out, false);

        REQUIRE(prompt_out.str().empty());
    }
}
