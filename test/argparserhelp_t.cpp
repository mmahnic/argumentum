// Copyright (c) 2018, 2019 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#include "../src/argparser.h"

#include <gtest/gtest.h>
#include <algorithm>

using namespace argparse;

TEST( ArgumentParserHelpTest, shouldAcceptArgumentHelpStrings )
{
   std::string str;
   std::vector<std::string> args;

   auto parser = ArgumentParser::create_unsafe();
   parser.add_argument( str, "-s" ).nargs( 1 ).help( "some value" );
   parser.add_argument( args, "args" ).minargs( 0 ).help( "some arguments" );

   auto res = parser.describe_argument( "-s" );
   EXPECT_EQ( "-s", res.short_name );
   EXPECT_EQ( "", res.long_name );
   EXPECT_EQ( "some value", res.help );
   EXPECT_FALSE( res.isPositional() );

   res = parser.describe_argument( "args" );
   EXPECT_EQ( "", res.short_name );
   EXPECT_EQ( "args", res.long_name );
   EXPECT_EQ( "some arguments", res.help );
   EXPECT_TRUE( res.isPositional() );

   EXPECT_THROW( parser.describe_argument( "--unknown" ), std::invalid_argument );
}

TEST( ArgumentParserHelpTest, shouldSetProgramName )
{
   auto parser = ArgumentParser::create_unsafe();
   parser.config().prog( "testing-testing" );

   auto& config = parser.getConfig();
   EXPECT_EQ( "testing-testing", config.program );
}

TEST( ArgumentParserHelpTest, shouldSetProgramDescription )
{
   auto parser = ArgumentParser::create_unsafe();
   parser.config().description( "An example." );

   auto& config = parser.getConfig();
   EXPECT_EQ( "An example.", config.description );
}

TEST( ArgumentParserHelpTest, shouldSetProgramUsage )
{
   auto parser = ArgumentParser::create_unsafe();
   parser.config().usage( "example [options] [arguments]" );

   auto& config = parser.getConfig();
   EXPECT_EQ( "example [options] [arguments]", config.usage );
}
