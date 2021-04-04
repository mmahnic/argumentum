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
