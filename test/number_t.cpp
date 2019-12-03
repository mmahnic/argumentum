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
