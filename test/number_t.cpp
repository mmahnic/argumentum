// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <cppargparse/argparse-s.h>

#include <algorithm>
#include <gtest/gtest.h>
#include <vector>

using namespace argparse;
using namespace testing;

TEST( NumberTest, shouldParseIntegerWithDecimalPrefix )
{
   int d;
   auto parser = argument_parser{};
   parser.add_argument( d, "number" ).nargs( 1 );

   auto res = parser.parse_args( { "0d12" } );
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_EQ( 12, d );
}

TEST( NumberTest, shouldParseLongWithDecimalPrefix )
{
   long d;
   auto parser = argument_parser{};
   parser.add_argument( d, "number" ).nargs( 1 );

   auto res = parser.parse_args( { "0d12" } );
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_EQ( 12, d );
}

TEST( ParseInt, shouldParsePositiveDecimal )
{
   EXPECT_EQ( 123, parse_int<int>( "123" ) );
}

TEST( ParseInt, shouldParsNegativeDecimal )
{
   EXPECT_EQ( -123, parse_int<int>( "-123" ) );
}

TEST( ParseInt, shouldParsePositiveDecimalWithPrefix )
{
   EXPECT_EQ( 123, parse_int<int>( "0d123" ) );
}

TEST( ParseInt, shouldParsNegativeDecimalWithPrefix )
{
   EXPECT_EQ( -123, parse_int<int>( "-0d123" ) );
}

TEST( ParseInt, shouldThrowOnInvalidInput )
{
   EXPECT_THROW( parse_int<int>( "abc" ), std::invalid_argument );
}

TEST( ParseInt, shouldThrowOnRangeViolation )
{
   EXPECT_THROW( parse_int<int>( "123456789123456789123456789" ), std::out_of_range );
}

TEST( ParseInt, shouldThrowOnShortRangeViolation )
{
   EXPECT_THROW( parse_int<short>( "99999" ), std::out_of_range );
}

// TODO: MANY tests for parse_int edge cases for base 10

TEST( ParseInt, shouldParsePositiveHexadecimalWithPrefix )
{
   EXPECT_EQ( 37312, parse_int<int>( "0x91c0" ) );
}

TEST( ParseInt, shouldParsNegativeHexadecimalWithPrefix )
{
   EXPECT_EQ( -64222, parse_int<int>( "-0xfade" ) );
}

// TODO: MANY tests for parse_int edge cases for base 16

TEST( ParseInt, shouldParsePositiveOctadecimalWithPrefix )
{
   EXPECT_EQ( 375, parse_int<int>( "0o567" ) );
}

TEST( ParseInt, shouldParsNegativeOctadecimalWithPrefix )
{
   EXPECT_EQ( -501, parse_int<int>( "-0o765" ) );
}

// TODO: MANY tests for parse_int edge cases for base 8

TEST( ParseInt, shouldParsePositiveBinaryWithPrefix )
{
   EXPECT_EQ( 27, parse_int<int>( "0b11011" ) );
}

TEST( ParseInt, shouldParsNegativeBinaryWithPrefix )
{
   EXPECT_EQ( -85, parse_int<int>( "-0b1010101" ) );
}

// TODO: MANY tests for parse_int edge cases for base 2
