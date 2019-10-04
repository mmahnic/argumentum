// Copyright (c) 2018, 2019 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#include "../src/argparser.h"
#include "../src/helpformatter.h"

#include <algorithm>
#include <gtest/gtest.h>
#include <sstream>

using namespace argparse;

namespace {
static const bool KEEPEMPTY = true;
std::vector<std::string_view> splitLines( std::string_view text, bool keepEmpty = false )
{
   std::vector<std::string_view> output;
   size_t start = 0;
   auto delims = "\n\r";

   auto isWinEol = [&text]( auto pos ) { return text[pos] == '\r' && text[pos + 1] == '\n'; };

   while ( start < text.size() ) {
      const auto stop = text.find_first_of( delims, start );

      if ( keepEmpty || start != stop )
         output.emplace_back( text.substr( start, stop - start ) );

      if ( stop == std::string_view::npos )
         break;

      start = stop + ( isWinEol( stop ) ? 2 : 1 );
   }

   return output;
}
}   // namespace

TEST( ArgumentParserHelpTest, shouldAcceptArgumentHelpStrings )
{
   std::string str;
   std::vector<std::string> args;

   auto parser = argument_parser{};
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
   auto parser = argument_parser{};
   parser.config().program( "testing-testing" );

   auto& config = parser.getConfig();
   EXPECT_EQ( "testing-testing", config.program );
}

TEST( ArgumentParserHelpTest, shouldSetProgramDescription )
{
   auto parser = argument_parser{};
   parser.config().description( "An example." );

   auto& config = parser.getConfig();
   EXPECT_EQ( "An example.", config.description );
}

TEST( ArgumentParserHelpTest, shouldSetProgramUsage )
{
   auto parser = argument_parser{};
   parser.config().usage( "example [options] [arguments]" );

   auto& config = parser.getConfig();
   EXPECT_EQ( "example [options] [arguments]", config.usage );
}

TEST( ArgumentParserHelpTest, shouldReturnDescriptionsOfAllArguments )
{
   std::string str;
   long depth;
   std::vector<std::string> args;

   auto parser = argument_parser{};
   parser.add_argument( str, "-s" ).nargs( 1 ).help( "some string" );
   parser.add_argument( depth, "-d", "--depth" ).nargs( 1 ).help( "some depth" );
   parser.add_argument( args, "args" ).minargs( 0 ).help( "some arguments" );

   auto descrs = parser.describe_arguments();
   EXPECT_EQ( 3, descrs.size() );
   EXPECT_EQ( 1, std::count_if( std::begin( descrs ), std::end( descrs ), []( auto&& d ) {
      return d.is_positional();
   } ) );
}

namespace {

class TestOptions : public argparse::Options
{
public:
   std::string str;
   long depth;
   long width;
   std::vector<std::string> args;

public:
   void add_arguments( argument_parser& parser ) override
   {
      parser.config()
            .program( "testing-format" )
            .description( "Format testing." )
            .usage( "testing-format [options]" )
            .epilog( "More about testing." );

      parser.add_argument( str, "-s" ).nargs( 1 ).help( "some string" );
      parser.add_argument( depth, "-d", "--depth" ).nargs( 1 ).help( "some depth" );
      parser.add_argument( width, "", "--width" ).nargs( 1 ).help( "some width" );
      parser.add_argument( args, "args" ).minargs( 0 ).help( "some arguments" );
   }
};

template<typename P, typename F>
std::string getTestHelp( P&& parser, F&& formatter )
{
   std::stringstream strout;
   formatter.format( parser, strout );
   return strout.str();
}

std::string getTestHelp()
{
   auto parser = argument_parser{};
   auto pOpt = std::make_shared<TestOptions>();
   parser.add_arguments( pOpt );

   return getTestHelp( parser, HelpFormatter() );
}

TEST( ArgumentParserHelpTest, shouldOutputHelpToStream )
{
   auto help = getTestHelp();

   auto parts = std::vector<std::string>{ "testing-format", "Format testing.",
      "testing-format [options]", "-s", "some string", "-d", "--depth", "some depth", "--width",
      "some width", "args", "some arguments", "More about testing." };

   for ( auto& p : parts )
      EXPECT_NE( std::string::npos, help.find( p ) ) << "Missing: " << p;
}

TEST( ArgumentParserHelpTest, shouldFormatDescriptionsToTheSameColumn )
{
   int dummy;
   auto parser = argument_parser{};
   parser.add_argument( dummy, "-s", "--parameter" ).nargs( 0 ).help( "some string" );
   parser.add_argument( dummy, "-x", "--parameterX" ).nargs( 0 ).help( "some depth" );
   parser.add_argument( dummy, "-y", "--parameterXX" ).nargs( 0 ).help( "some width" );
   parser.add_argument( dummy, "args" ).nargs( 0 ).help( "some arguments" );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help );

   auto parts =
         std::vector<std::string>{ "some string", "some depth", "some width", "some arguments" };

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
}   // namespace

TEST( ArgumentParserHelpTest, shouldSetHelpEpilog )
{
   auto parser = argument_parser{};
   parser.config().epilog( "This comes after help." );

   auto& config = parser.getConfig();
   EXPECT_EQ( "This comes after help.", config.epilog );
}

TEST( ArgumentParserHelpTest, shouldReformatLongDescriptions )
{
   std::string loremipsum;
   auto parser = argument_parser{};
   parser.add_argument( loremipsum, "--lorem-ipsum" )
         .nargs( 1 )
         .help( "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
                "sed do eiusmod tempor incididunt ut labore et dolore magna "
                "aliqua. Ut enim ad minim veniam, quis nostrud exercitation "
                "ullamco laboris nisi ut aliquip ex ea commodo consequat." );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help );

   for ( auto line : lines )
      EXPECT_GE( 60, line.size() );
}

TEST( ArgumentParserHelpTest, shouldLimitTheWidthOfReformattedDescriptions )
{
   std::string loremipsum;
   auto parser = argument_parser{};
   parser.add_argument( loremipsum, "--lorem-ipsum-x-with-a-longer-name" )
         .nargs( 1 )
         .help( "xxxxx xxxxx xxxxx xxx xxxx, xxxxxxxxxxx xxxxxxxxxx xxxx, "
                "xxx xx xxxxxxx xxxxxx xxxxxxxxxx xx xxxxxx xx xxxxxx xxxxx "
                "xxxxxx. xx xxxx xx xxxxx xxxxxx, xxxx xxxxxxx xxxxxxxxxxxx "
                "xxxxxxx xxxxxxx xxxx xx xxxxxxx xx xx xxxxxxx xxxxxxxxx." );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help );

   for ( auto line : lines ) {
      EXPECT_GE( 60, line.size() );
      auto pos = line.find( "xx" );
      if ( pos != std::string::npos ) {
         EXPECT_LE( 20, pos );
         EXPECT_GT( 22, pos );
      }
   }
}

TEST( ArgumentParserHelpTest, shouldKeepSourceParagraphsInDescriptions )
{
   std::string loremipsum;
   auto parser = argument_parser{};
   parser.add_argument( loremipsum, "--paragraph" ).nargs( 1 ).help( "xxxxx.\n\nyyyy" );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   int lx = -1;
   int ly = -1;
   int i = 0;
   for ( auto line : lines ) {
      if ( line.find( "xxxx" ) != std::string::npos )
         lx = i;
      if ( line.find( "yyyy" ) != std::string::npos )
         ly = i;
      ++i;
   }

   EXPECT_EQ( ly, lx + 2 );
}

TEST( ArgumentParserHelpTest, shouldDescribeOptionArguments )
{
   std::string str;
   auto parser = argument_parser{};
   parser.add_argument( str, "-a" ).nargs( 2 );
   parser.add_argument( str, "--bees" ).minargs( 1 );
   parser.add_argument( str, "-c" ).minargs( 0 );
   parser.add_argument( str, "-d" ).minargs( 2 );
   parser.add_argument( str, "-e" ).maxargs( 3 );
   parser.add_argument( str, "-f" ).maxargs( 1 );

   auto res = parser.describe_argument( "-a" );
   EXPECT_EQ( "A A", res.arguments );

   res = parser.describe_argument( "--bees" );
   EXPECT_EQ( "BEES [BEES ...]", res.arguments );

   res = parser.describe_argument( "-c" );
   EXPECT_EQ( "[C ...]", res.arguments );

   res = parser.describe_argument( "-d" );
   EXPECT_EQ( "D D [D ...]", res.arguments );

   res = parser.describe_argument( "-e" );
   EXPECT_EQ( "[E {0..3}]", res.arguments );

   res = parser.describe_argument( "-f" );
   EXPECT_EQ( "[F]", res.arguments );
}

TEST( ArgumentParserHelpTest, shouldOutputOptionArguments )
{
   std::string str;
   auto parser = argument_parser{};
   parser.add_argument( str, "--bees" ).minargs( 1 );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   for ( auto line : lines ) {
      auto optpos = line.find( "--bees" );
      if ( optpos == std::string::npos )
         continue;

      auto argspos = line.find( "BEES [BEES ...]" );
      ASSERT_NE( std::string::npos, argspos );
      EXPECT_LT( optpos, argspos );
   }
}

TEST( ArgumentParserHelpTest, shouldChangeOptionMetavarName )
{
   std::string str;
   auto parser = argument_parser{};
   parser.add_argument( str, "--bees" ).minargs( 1 ).metavar( "WORK" );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   for ( auto line : lines ) {
      auto optpos = line.find( "--bees" );
      if ( optpos == std::string::npos )
         continue;

      auto argspos = line.find( "WORK [WORK ...]" );
      ASSERT_NE( std::string::npos, argspos );
      EXPECT_LT( optpos, argspos );
   }
}
