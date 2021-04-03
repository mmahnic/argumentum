// Copyright (c) 2021 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <argumentum/argparse.h>

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace argumentum;

TEST( ForwardParam, shouldCollectSingleParam )
{
   std::vector<std::string> forward;

   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( forward, "--forward", "-f" ).forward( true );

   parser.parse_args( { "--forward,--one", "-f,--two" } );

   ASSERT_EQ( 2, forward.size() );
   EXPECT_EQ( "--one", forward.front() );
   EXPECT_EQ( "--two", forward.back() );
}
