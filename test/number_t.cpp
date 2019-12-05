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

// TODO: MANY tests for parse_int edge cases
