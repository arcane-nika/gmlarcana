#include "catch/catch.hpp"
#include "ogm/common/util.hpp"

#include <iostream>

#include "ogm/sys/util_sys.hpp"

using namespace ogm;

TEST_CASE( "ends_with" )
{
    REQUIRE(ends_with("b/a.gml", ".gml"));
    REQUIRE(!ends_with("b/a.gml", ".ogm"));
    REQUIRE(!ends_with("b/a.gml", "."));
    REQUIRE(!ends_with("b/a.gml", ".."));
}

TEST_CASE( "native path normalization" )
{
    REQUIRE(normalize_native_path("") == "");
    REQUIRE(
        normalize_native_path("relative")
        == "relative"
    );

#ifdef _WIN32
    REQUIRE(
        normalize_native_path(
            R"(C:\Users\\\strange_path/xyz)"
        )
        == R"(C:\Users\strange_path\xyz)"
    );

    REQUIRE(
        normalize_native_path(
            R"(\\server///share\\folder)"
        )
        == R"(\\server\share\folder)"
    );

    REQUIRE(
        normalize_native_path(R"(C:\\)")
        == R"(C:\)"
    );

    REQUIRE(
        normalize_native_path(
            R"(C:\a//./b\\..\c)"
        )
        == R"(C:\a\.\b\..\c)"
    );
#else
    REQUIRE(
        normalize_native_path(
            R"(/home/etc//hi\xyz)"
        )
        == "/home/etc/hi/xyz"
    );

    REQUIRE(
        normalize_native_path("/home//")
        == "/home/"
    );

    REQUIRE(
        normalize_native_path(
            R"(/a//./b\../c)"
        )
        == "/a/./b/../c"
    );
#endif

    std::string mixed_path =
        R"(one//two\\three)";

    REQUIRE(
        normalize_native_path(
            normalize_native_path(mixed_path)
        )
        == normalize_native_path(mixed_path)
    );
}