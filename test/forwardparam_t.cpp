// Copyright (c) 2021 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <argumentum/argparse.h>

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace argumentum;

TEST( ForwardParam, shouldCollectSingleParamFromLongOption )
{
   std::vector<std::string> forward;

   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( forward, "--forward" ).forward( true );

   auto res = parser.parse_args( { "--forward,--one", "--forward,--two" } );
   EXPECT_TRUE( static_cast<bool>( res ) );

   ASSERT_EQ( 2, forward.size() );
   EXPECT_EQ( "--one", forward.front() );
   EXPECT_EQ( "--two", forward.back() );
}

TEST( ForwardParam, shouldShouldFailWithEmptyForwardParam )
{
   std::vector<std::string> forward;

   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( forward, "--forward" ).forward( true );

   auto res = parser.parse_args( { "--forward," } );
   EXPECT_FALSE( static_cast<bool>( res ) );
}

TEST( ForwardParam, shouldShouldFailWhenForwardIsFalse )
{
   std::vector<std::string> forward;

   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( forward, "--forward" ).forward( false );

   auto res = parser.parse_args( { "--forward,--one" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
}

TEST( ForwardParam, shouldCollectMultipleParamsFromLongOption )
{
   std::vector<std::string> forward;

   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( forward, "--forward" ).forward( true );

   auto res = parser.parse_args( { "--forward,--one,first,second", "--forward,--two,third" } );
   EXPECT_TRUE( static_cast<bool>( res ) );

   ASSERT_EQ( 5, forward.size() );
   EXPECT_EQ( "--one", forward[0] );
   EXPECT_EQ( "first", forward[1] );
   EXPECT_EQ( "second", forward[2] );
   EXPECT_EQ( "--two", forward[3] );
   EXPECT_EQ( "third", forward[4] );
}

TEST( ForwardParam, shouldEscapeCommaInParams )
{
   std::vector<std::string> forward;

   // TODO: (maybe) Escaping of commas in forwareded arguments could ba a parser setting.
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( forward, "--forward" ).forward( true );

   auto res = parser.parse_args(
         { "--forward,--one,,combined", "--forward,,first-escaped,second,,combined" } );
   EXPECT_TRUE( static_cast<bool>( res ) );

   ASSERT_EQ( 3, forward.size() );
   EXPECT_EQ( "--one,combined", forward[0] );
   EXPECT_EQ( ",first-escaped", forward[1] );
   EXPECT_EQ( "second,combined", forward[2] );
}
