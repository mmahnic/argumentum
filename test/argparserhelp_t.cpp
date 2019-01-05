// Copyright (c) 2018, 2019 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#include "../src/argparser.h"
#include "../src/helpformatter.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <sstream>

using namespace argparse;

namespace {
std::vector<std::string_view> splitLines( std::string_view text )
{
    std::vector<std::string_view> output;
    size_t start = 0;
    auto delims = "\n\r";

    auto isWinEol = [&text]( auto pos ) {
       return text[pos] == '\r' && text[pos+1] == '\n';
    };

    while ( start < text.size() ) {
       const auto stop = text.find_first_of( delims, start );

       if ( start != stop )
          output.emplace_back( text.substr( start, stop-start ) );

       if ( stop == std::string_view::npos )
          break;

       start = stop + ( isWinEol( stop ) ? 2 : 1 );
    }

    return output;
}
}

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
   EXPECT_FALSE( res.is_positional() );

   res = parser.describe_argument( "args" );
   EXPECT_EQ( "", res.short_name );
   EXPECT_EQ( "args", res.long_name );
   EXPECT_EQ( "some arguments", res.help );
   EXPECT_TRUE( res.is_positional() );

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

TEST( ArgumentParserHelpTest, shouldReturnDescriptionsOfAllArguments )
{
   std::string str;
   long depth;
   std::vector<std::string> args;

   auto parser = ArgumentParser::create_unsafe();
   parser.add_argument( str, "-s" ).nargs( 1 ).help( "some string" );
   parser.add_argument( depth, "-d", "--depth" ).nargs( 1 ).help( "some depth" );
   parser.add_argument( args, "args" ).minargs( 0 ).help( "some arguments" );

   auto descrs = parser.describe_arguments();
   EXPECT_EQ( 3, descrs.size() );
   EXPECT_EQ( 1, std::count_if( std::begin( descrs ), std::end( descrs ),
            []( auto&& d ) { return d.is_positional(); } ) );
}

TEST( ArgumentParserHelpTest, shouldOutputHelpToStream )
{
   std::string str;
   long depth;
   std::vector<std::string> args;

   auto parser = ArgumentParser::create_unsafe();
   parser.config()
      .prog( "testing-format" )
      .description( "Format testing." )
      .usage( "testing-format [options]" );

   parser.add_argument( str, "-s" ).nargs( 1 ).help( "some string" );
   parser.add_argument( depth, "-d", "--depth" ).nargs( 1 ).help( "some depth" );
   parser.add_argument( args, "args" ).minargs( 0 ).help( "some arguments" );

   std::stringstream strout;
   auto formatter = HelpFormatter();
   formatter.format( parser, strout );
   auto help = strout.str();

   auto parts = std::vector<std::string>{ "testing-format", "Format testing.",
      "testing-format [options]", "-s", "some string", "-d", "--depth",
      "some depth", "args", "some arguments" };

   for ( auto& p : parts )
      EXPECT_NE( std::string::npos, help.find( p ) ) << "Missing: " << p;
}

TEST( ArgumentParserHelpTest, shouldFormatDescriptionsToTheSameColumn )
{
   std::string str;
   long depth;
   long width;
   std::vector<std::string> args;

   auto parser = ArgumentParser::create_unsafe();
   parser.config()
      .prog( "testing-format" )
      .description( "Format testing." )
      .usage( "testing-format [options]" );

   parser.add_argument( str, "-s" ).nargs( 1 ).help( "some string" );
   parser.add_argument( depth, "-d", "--depth" ).nargs( 1 ).help( "some depth" );
   parser.add_argument( width, "", "--width" ).nargs( 1 ).help( "some width" );
   parser.add_argument( args, "somestuff" ).minargs( 0 ).help( "some arguments" );

   std::stringstream strout;
   auto formatter = HelpFormatter();
   formatter.format( parser, strout );
   auto help = strout.str();
   auto helpLines = splitLines(help);

   auto parts = std::vector<std::string>{ "some string", "some depth", "some width",
      "some arguments" };

   auto findColumn = [&helpLines]( auto&& text ) -> size_t {
      for ( auto&& l : helpLines ) {
         auto pos = l.find( text );
         if ( pos != std::string::npos )
            return pos;
      }
      return std::string::npos;
   };

   auto column = findColumn( parts[0] );
   ASSERT_NE( std::string::npos, column );
   for ( auto& p : parts )
      EXPECT_EQ( column, findColumn( p ) ) << "Not aligned: " << p;
}

TEST( ArgumentParserHelpTest, shouldSetHelpEpilog )
{
   auto parser = ArgumentParser::create_unsafe();
   parser.config().epilog( "This comes after help." );

   auto& config = parser.getConfig();
   EXPECT_EQ( "This comes after help.", config.epilog );
}

