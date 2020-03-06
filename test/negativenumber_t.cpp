// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <argumentum/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>

using namespace argumentum;
using namespace testing;

// A negative number looks like a short option.  The parser should detect if the
// argument is really an option or a negative number.  Rules when an argument
// looks like a negative number:
//   - If an option is active and accepts arguments, the argument is a number.
//   - If an option is not active and '--' was not seen at this point, the
//   argument is a multi-option but only if the first character represents an
//   existing option.  Otherwise the argument is a numeric positional.

TEST( NegativeNumberTest, shouldDistinguishNegativeNumbersFromOptions )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   int i;
   params.add_parameter( i, "--num" ).nargs( 1 );

   auto res = parser.parse_args( { "--num", "-5" } );
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_EQ( -5, i );
}

TEST( NegativeNumberTest, shouldGivePrecedenceToOptionValueOverDigitOption )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   int i;
   int d;
   params.add_parameter( i, "--num" ).nargs( 1 );
   params.add_parameter( d, "-5" ).nargs( 0 ).absent( 100 );

   auto res = parser.parse_args( { "--num", "-5" } );
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_EQ( -5, i );
   EXPECT_EQ( 100, d );
}

TEST( NegativeNumberTest, shouldDistinguishNegativeNumbersFromOptionsInPositionalParams )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   int i;
   int j;
   params.add_parameter( i, "--num" ).nargs( 1 );
   params.add_parameter( j, "number" ).nargs( 1 );

   auto res = parser.parse_args( { "--num", "-5", "-6" } );
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_EQ( -5, i );
   EXPECT_EQ( -6, j );
}

TEST( NegativeNumberTest, shouldGivePrecedenceToDigitOptionOverPositionalParam )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   int i;
   int j;
   int d;
   params.add_parameter( i, "--num" ).nargs( 1 );
   params.add_parameter( j, "number" ).nargs( 1 );
   params.add_parameter( d, "-6" ).nargs( 0 ).absent( 100 ).flagValue( "60" );

   auto res = parser.parse_args( { "--num", "-5", "-6" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_EQ( -5, i );
   EXPECT_EQ( 0, j );
   EXPECT_EQ( 60, d );
}

TEST( NegativeNumberTest, shouldMakePositionalParamAfterDashDash )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   int i;
   int j;
   int d;
   params.add_parameter( i, "--num" ).nargs( 1 );
   params.add_parameter( j, "number" ).nargs( 1 );
   params.add_parameter( d, "-6" ).nargs( 0 ).absent( 100 ).flagValue( "60" );

   auto res = parser.parse_args( { "--num", "-5", "--", "-6" } );
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_EQ( -5, i );
   EXPECT_EQ( -6, j );
   EXPECT_EQ( 100, d );
}
