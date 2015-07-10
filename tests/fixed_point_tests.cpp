// File: fixed_point_tests.cpp

#include <string>
#include <iostream>

#include "catch.hpp"

#include "../src/fixed.h"

TEST_CASE("multiplication", "[fixed-point]")
{
    bool overflow = false;

    CHECK(fixed_mult(10, 20, &overflow) == 2);
    REQUIRE(overflow == false);

    CHECK(fixed_mult(1234, 5739, &overflow) == 70819);
    REQUIRE(overflow == false);

    CHECK(fixed_mult(-1234, 5739, &overflow) == -70819);
    REQUIRE(overflow == false);

    CHECK(fixed_mult(1234, -5739, &overflow) == -70819);
    REQUIRE(overflow == false);

    CHECK(fixed_mult(-1234, -5739, &overflow) == 70819);
    REQUIRE(overflow == false);

    CHECK(fixed_mult(99900, 99900, &overflow) == 99800100);
    REQUIRE(overflow == false);

    CHECK(fixed_mult(-99900, 99900, &overflow) == -99800100);
    REQUIRE(overflow == false);

    CHECK(fixed_mult(99900, -99900, &overflow) == -99800100);
    REQUIRE(overflow == false);

    CHECK(fixed_mult(-99900, -99900, &overflow) == 99800100);
    REQUIRE(overflow == false);

    fixed_mult(999000, 999000, &overflow);
    REQUIRE(overflow == true);
    overflow = false;
}

TEST_CASE("division", "[fixed-point]")
{
    CHECK(fixed_div(1234, 5739) == 21);
    CHECK(fixed_div(1234, -5739) == -21);
    CHECK(fixed_div(-1234, 5739) == -21);
    CHECK(fixed_div(-1234, -5739) == 21);
    CHECK(fixed_div(1000, 50) == 2000);
    CHECK(fixed_div(FIXED_MAX, 50) == 0);
}

TEST_CASE("addition", "[fixed-point]")
{
    bool overflow = false;

    CHECK(fixed_add(1234, 5739, &overflow) == 6973);
    REQUIRE(overflow == false);

    CHECK(fixed_add(1234, -5739, &overflow) == -4505);
    REQUIRE(overflow == false);

    CHECK(fixed_add(-1234, 5739, &overflow) == 4505);
    REQUIRE(overflow == false);

    CHECK(fixed_add(-1234, -5739, &overflow) == -6973);
    REQUIRE(overflow == false);

    CHECK(fixed_add(FIXED_MAX-1, 1, &overflow) == FIXED_MAX);
    REQUIRE(overflow == false);

    fixed_add(FIXED_MAX, 1, &overflow);
    REQUIRE(overflow == true);
    overflow = false;
}

TEST_CASE("subtraction", "[fixed-point]")
{
    bool overflow = false;

    CHECK(fixed_subt(1234, 5739, &overflow) == -4505);
    REQUIRE(overflow == false);

    CHECK(fixed_subt(1234, -5739, &overflow) == 6973);
    REQUIRE(overflow == false);

    CHECK(fixed_subt(-1234, 5739, &overflow) == -6973);
    REQUIRE(overflow == false);

    CHECK(fixed_subt(-1234, -5739, &overflow) == 4505);
    REQUIRE(overflow == false);
}

TEST_CASE("exponent", "[fixed-point]")
{
    bool overflow = false;

    CHECK(fixed_pow(12300, 0, &overflow) == 100);
    REQUIRE(overflow == false);

    CHECK(fixed_pow(-12300, 0, &overflow) == 100);
    REQUIRE(overflow == false);

    CHECK(fixed_pow(0, 0, &overflow) == 100);
    REQUIRE(overflow == false);

    CHECK(fixed_pow(200, 3, &overflow) == 800);
    REQUIRE(overflow == false);

    CHECK(fixed_pow(-200, 3, &overflow) == -800);
    REQUIRE(overflow == false);

    CHECK(fixed_pow(250, 2, &overflow) == 625);
    REQUIRE(overflow == false);

    CHECK(fixed_pow(-250, 2, &overflow) == 625);
    REQUIRE(overflow == false);

    CHECK(fixed_pow(1000, -2, &overflow) == 1);
    REQUIRE(overflow == false);
}

TEST_CASE("text representation", "[fixed-point]")
{
    char buffer[64];
    std::string repr;

    repr.assign(fixed_repr(1234, buffer, sizeof(buffer)));
    CHECK(repr == "12.34");

    repr.assign(fixed_repr(-1234, buffer, sizeof(buffer)));
    CHECK(repr == "-12.34");

    repr.assign(fixed_repr(0, buffer, sizeof(buffer)));
    CHECK(repr == "0");

    repr.assign(fixed_repr(1, buffer, sizeof(buffer)));
    CHECK(repr == "0.01");

    repr.assign(fixed_repr(10, buffer, sizeof(buffer)));
    CHECK(repr == "0.1");

    repr.assign(fixed_repr(230, buffer, sizeof(buffer)));
    CHECK(repr == "2.3");

    repr.assign(fixed_repr(21, buffer, sizeof(buffer)));
    CHECK(repr == "0.21");

    repr.assign(fixed_repr(-21, buffer, sizeof(buffer)));
    CHECK(repr == "-0.21");
}

TEST_CASE("conversion from string", "[fixed-point]")
{
    bool overflow = false;

    CHECK(str_to_fixed("123.45", &overflow) == 12345);
    REQUIRE(overflow == false);

    CHECK(str_to_fixed("123", &overflow) == 12300);
    REQUIRE(overflow == false);

    CHECK(str_to_fixed("123.00", &overflow) == 12300);
    REQUIRE(overflow == false);

    CHECK(str_to_fixed("123.10", &overflow) == 12310);
    REQUIRE(overflow == false);

    CHECK(str_to_fixed("123.1", &overflow) == 12310);
    REQUIRE(overflow == false);

    CHECK(str_to_fixed("123.01", &overflow) == 12301);
    REQUIRE(overflow == false);

    CHECK(str_to_fixed("-123.01", &overflow) == -12301);
    REQUIRE(overflow == false);

    CHECK(str_to_fixed("-0.21", &overflow) == -21);
    REQUIRE(overflow == false);

    CHECK(str_to_fixed("0.21", &overflow) == 21);
    REQUIRE(overflow == false);

    CHECK(str_to_fixed("9.00", &overflow) == 900);
    REQUIRE(overflow == false);

    CHECK(str_to_fixed("21474836.47", &overflow) == 2147483647);
    REQUIRE(overflow == false);

    CHECK(str_to_fixed("2147483.647", &overflow) == 214748364);
    REQUIRE(overflow == false);

    str_to_fixed("214748364.7", &overflow);
    REQUIRE(overflow == true);
    overflow = false;

    str_to_fixed("21474837.48", &overflow);
    REQUIRE(overflow == true);
    overflow = false;
}
