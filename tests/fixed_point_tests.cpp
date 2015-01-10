// File: fixed_point_tests.cpp

#include <string>
#include <iostream>

#include <catch.hpp>

#include "../src/fixed.h"

TEST_CASE("multiplication", "[fixed-point]")
{
    CHECK(fixed_mult(10, 20) == 2);
    CHECK(fixed_mult(1234, 5739) == 70819);
    CHECK(fixed_mult(-1234, 5739) == -70819);
    CHECK(fixed_mult(1234, -5739) == -70819);
    CHECK(fixed_mult(-1234, -5739) == 70819);
}

TEST_CASE("division", "[fixed-point]")
{
    CHECK(fixed_div(1234, 5739) == 21);
    CHECK(fixed_div(1234, -5739) == -21);
    CHECK(fixed_div(-1234, 5739) == -21);
    CHECK(fixed_div(-1234, -5739) == 21);

}

TEST_CASE("addition", "[fixed-point]")
{
    CHECK(fixed_add(1234, 5739) == 6973);
    CHECK(fixed_add(1234, -5739) == -4505);
    CHECK(fixed_add(-1234, 5739) == 4505);
    CHECK(fixed_add(-1234, -5739) == -6973);

}

TEST_CASE("subtraction", "[fixed-point]")
{
    CHECK(fixed_subt(1234, 5739) == -4505);
    CHECK(fixed_subt(1234, -5739) == 6973);
    CHECK(fixed_subt(-1234, 5739) == -6973);
    CHECK(fixed_subt(-1234, -5739) == 4505);
}

TEST_CASE("exponent", "[fixed-point]")
{
    CHECK(fixed_pow(12300, 0) == 100);
    CHECK(fixed_pow(-12300, 0) == 100);
    CHECK(fixed_pow(0, 0) == 100);
    CHECK(fixed_pow(200, 3) == 800);
    CHECK(fixed_pow(-200, 3) == -800);
    CHECK(fixed_pow(250, 2) == 625);
    CHECK(fixed_pow(-250, 2) == 625);
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
    CHECK(str_to_fixed("123.45") == 12345);
    CHECK(str_to_fixed("123") == 12300);
    CHECK(str_to_fixed("123.00") == 12300);
    CHECK(str_to_fixed("123.10") == 12310);
    CHECK(str_to_fixed("123.1") == 12310);
    CHECK(str_to_fixed("123.01") == 12301);
    CHECK(str_to_fixed("-123.01") == -12301);
    CHECK(str_to_fixed("-0.21") == -21);
    CHECK(str_to_fixed("0.21") == 21);
}
