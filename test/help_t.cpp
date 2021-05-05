// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "testutil.h"

#include <argumentum/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>
#include <sstream>

using namespace argumentum;
using namespace testutil;

TEST( ArgumentParserHelpTest, shouldAcceptArgumentHelpStrings )
{
   std::string str;
   std::vector<std::string> args;

   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "-s" ).nargs( 1 ).help( "some value" );
   params.add_parameter( args, "args" ).minargs( 0 ).help( "some arguments" );

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
   EXPECT_EQ( "testing-testing", config.program() );
}

TEST( ArgumentParserHelpTest, shouldSetProgramDescription )
{
   auto parser = argument_parser{};
   parser.config().description( "An example." );

   auto& config = parser.getConfig();
   EXPECT_EQ( "An example.", config.description() );
}

TEST( ArgumentParserHelpTest, shouldSetProgramUsage )
{
   auto parser = argument_parser{};
   parser.config().usage( "example [options] [arguments]" );

   auto& config = parser.getConfig();
   EXPECT_EQ( "example [options] [arguments]", config.usage() );
}

TEST( ArgumentParserHelpTest, shouldReturnDescriptionsOfAllArguments )
{
   std::string str;
   long depth;
   std::vector<std::string> args;

   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "-s" ).nargs( 1 ).help( "some string" );
   params.add_parameter( depth, "-d", "--depth" ).nargs( 1 ).help( "some depth" );
   params.add_parameter( args, "args" ).minargs( 0 ).help( "some arguments" );

   auto descrs = parser.describe_arguments();
   EXPECT_EQ( 3, descrs.size() );
   EXPECT_EQ( 1, std::count_if( std::begin( descrs ), std::end( descrs ), []( auto&& d ) {
      return d.is_positional();
   } ) );
}

namespace {

class TestOptions : public argumentum::Options
{
public:
   std::string str;
   long depth;
   long width;
   std::vector<std::string> args;

public:
   void add_parameters( ParameterConfig& params ) override
   {
      params.add_parameter( str, "-s" ).nargs( 1 ).help( "some string" );
      params.add_parameter( depth, "-d", "--depth" ).nargs( 1 ).help( "some depth" );
      params.add_parameter( width, "", "--width" ).nargs( 1 ).help( "some width" );
      params.add_parameter( args, "args" ).minargs( 0 ).help( "some arguments" );
   }
};
}   // namespace

TEST( ArgumentParserHelpTest, shouldOutputHelpToStream )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config()
         .program( "testing-format" )
         .description( "Format testing." )
         .usage( "testing-format [options]" )
         .epilog( "More about testing." );

   auto pOpt = std::make_shared<TestOptions>();
   params.add_parameters( pOpt );
   auto help = getTestHelp( parser, HelpFormatter() );

   auto parts = std::vector<std::string>{ "testing-format", "Format testing.",
      "testing-format [options]", "-s", "some string", "-d", "--depth", "some depth", "--width",
      "some width", "args", "some arguments", "More about testing." };

   for ( auto& p : parts )
      EXPECT_TRUE( strHasText( help, p ) ) << "Missing: " << p;
}

TEST( ArgumentParserHelpTest, shouldFormatDescriptionsToTheSameColumn )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( dummy, "-s", "--parameter" ).nargs( 0 ).help( "some string" );
   params.add_parameter( dummy, "-x", "--parameterX" ).nargs( 0 ).help( "some depth" );
   params.add_parameter( dummy, "-y", "--parameterXX" ).nargs( 0 ).help( "some width" );
   params.add_parameter( dummy, "args" ).nargs( 0 ).help( "some arguments" );

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

TEST( ArgumentParserHelpTest, shouldSetHelpEpilog )
{
   auto parser = argument_parser{};
   parser.config().epilog( "This comes after help." );

   auto& config = parser.getConfig();
   EXPECT_EQ( "This comes after help.", config.epilog() );
}

TEST( ArgumentParserHelpTest, shouldReformatLongDescriptions )
{
   std::string loremipsum;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( loremipsum, "--lorem-ipsum" )
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
   auto params = parser.params();
   params.add_parameter( loremipsum, "--lorem-ipsum-x-with-a-longer-name" )
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
   auto params = parser.params();
   params.add_parameter( loremipsum, "--paragraph" ).nargs( 1 ).help( "xxxxx.\n\nyyyy" );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   int lx = -1;
   int ly = -1;
   int i = 0;
   for ( auto line : lines ) {
      if ( strHasText( line, "xxxx" ) )
         lx = i;
      if ( strHasText( line, "yyyy" ) )
         ly = i;
      ++i;
   }

   EXPECT_EQ( ly, lx + 2 );
}

TEST( ArgumentParserHelpTest, shouldDescribeOptionArguments )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "-a" ).nargs( 2 );
   params.add_parameter( str, "--bees" ).minargs( 1 );
   params.add_parameter( str, "-c" ).minargs( 0 );
   params.add_parameter( str, "-d" ).minargs( 2 );
   params.add_parameter( str, "-e" ).maxargs( 3 );
   params.add_parameter( str, "-f" ).maxargs( 1 );

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
   auto params = parser.params();
   params.add_parameter( str, "--bees" ).minargs( 1 );

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
   auto params = parser.params();
   params.add_parameter( str, "--bees" ).minargs( 1 ).metavar( "WORK" );

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

TEST( ArgumentParserHelpTest, shouldDescribePositionalArguments )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "a" ).nargs( 2 );
   params.add_parameter( str, "bees" ).minargs( 1 );
   params.add_parameter( str, "c" ).minargs( 0 );
   params.add_parameter( str, "d" ).minargs( 2 );
   params.add_parameter( str, "e" ).maxargs( 3 );
   params.add_parameter( str, "f" ).maxargs( 1 );

   auto res = parser.describe_argument( "a" );
   EXPECT_EQ( "a a", res.arguments );

   res = parser.describe_argument( "bees" );
   EXPECT_EQ( "bees [bees ...]", res.arguments );

   res = parser.describe_argument( "c" );
   EXPECT_EQ( "[c ...]", res.arguments );

   res = parser.describe_argument( "d" );
   EXPECT_EQ( "d d [d ...]", res.arguments );

   res = parser.describe_argument( "e" );
   EXPECT_EQ( "[e {0..3}]", res.arguments );

   res = parser.describe_argument( "f" );
   EXPECT_EQ( "[f]", res.arguments );
}

TEST( ArgumentParserHelpTest, shouldOutputPositionalArguments )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "aaa" ).nargs( 3 ).help( "Triple a." );
   parser.config().usage( "test" );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   bool hasA = false;
   bool hasAA = false;
   for ( auto line : lines ) {
      if ( line.find( "aaa" ) != std::string::npos )
         hasA = true;
      if ( line.find( "aaa aaa" ) != std::string::npos )
         hasAA = true;
   }
   EXPECT_TRUE( hasA );
   EXPECT_FALSE( hasAA );
}

// TODO: The metavar for the positional argument must be the same as the name of
// the option, oterwise we loose the connection between usage and description.
//   a) forbid / ignore metavar()
//   b) also change the name in metavar()
//   c) display name and metavar in description: name(metavar)

TEST( ArgumentParserHelpTest, shouldSplitOptionalAndMandatoryArguments )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( dummy, "--yes" ).nargs( 0 ).required( true ).help( "req:true" );
   params.add_parameter( dummy, "--no" ).nargs( 0 ).required( false ).help( "req:false" );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help );

   bool hasRequired = false;
   bool hasOptional = false;
   enum class EBlock { other, required, optional };
   auto block = EBlock::required;
   std::map<std::string, EBlock> found;

   for ( auto line : helpLines ) {
      if ( strHasText( line, "optional arguments" ) ) {
         hasOptional = true;
         block = EBlock::optional;
      }
      if ( strHasText( line, "required arguments" ) ) {
         hasRequired = true;
         block = EBlock::required;
      }
      for ( auto param : { "--yes", "--no" } ) {
         if ( strHasText( line, param ) )
            found[param] = block;
      }
   }

   EXPECT_TRUE( hasOptional );
   EXPECT_TRUE( hasRequired );
   EXPECT_EQ( EBlock::required, found["--yes"] );
   EXPECT_EQ( EBlock::optional, found["--no"] );
}

TEST( ArgumentParserHelpTest, shouldSortParametersByGroups )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().usage( "test" );

   params.add_parameter( dummy, "--no" ).nargs( 0 ).required( false ).help( "default:no" );
   params.add_parameter( dummy, "--yes" ).nargs( 0 ).required( true ).help( "default:yes" );
   params.add_parameter( dummy, "positional" ).nargs( 0 ).help( "default:positional" );
   params.add_group( "simple" );
   params.add_parameter( dummy, "--first" ).nargs( 0 ).help( "simple:first" );
   params.add_parameter( dummy, "--second" ).nargs( 0 ).help( "simple:second" );
   params.add_parameter( dummy, "simplicity" ).help( "simple:simplicity" );
   params.add_exclusive_group( "exclusive" );
   params.add_parameter( dummy, "--on" ).nargs( 0 ).help( "exclusive:on" );
   params.add_parameter( dummy, "--off" ).nargs( 0 ).help( "exclusive:off" );
   params.add_group( "last" );
   params.add_parameter( dummy, "--last" ).nargs( 0 ).help( "last:last" );
   params.end_group();
   params.add_parameter( dummy, "--maybe" ).nargs( 0 ).required( false ).help( "default:maybe" );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help );

   auto opts = std::set<std::string>{ "--no", "--yes", "positional", "--first", "--second",
      "simplicity", "--on", "--off", "--last", "--maybe" };
   std::map<std::string, int> foundOpts;
   int i = 0;
   for ( auto line : helpLines ) {
      for ( auto opt : opts ) {
         if ( strHasText( line, opt ) ) {
            foundOpts[opt] = i;
            opts.erase( opt );
            break;
         }
      }
      ++i;
   }

   EXPECT_EQ( 0, opts.size() );   // all options were found

   // Expected group order: Positional, Required, Optional; by name: Exclusive, Last, Simple
   EXPECT_LT( foundOpts["positional"], foundOpts["--yes"] );
   EXPECT_LT( foundOpts["--yes"], foundOpts["--no"] );
   EXPECT_LT( foundOpts["--yes"], foundOpts["--maybe"] );
   EXPECT_LT( foundOpts["--no"], foundOpts["--off"] );
   EXPECT_LT( foundOpts["--maybe"], foundOpts["--off"] );
   EXPECT_LT( foundOpts["--maybe"], foundOpts["--on"] );
   EXPECT_LT( foundOpts["--on"], foundOpts["--off"] );
   EXPECT_LT( foundOpts["--on"], foundOpts["--last"] );
   EXPECT_LT( foundOpts["--off"], foundOpts["--last"] );
   EXPECT_LT( foundOpts["--last"], foundOpts["simplicity"] );
   EXPECT_LT( foundOpts["simplicity"], foundOpts["--first"] );
   EXPECT_LT( foundOpts["--first"], foundOpts["--second"] );
}

TEST( ArgumentParserHelpTest, shouldOutputGroupTitle )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().description( "Should output group title." );
   params.add_parameter( dummy, "--default" ).nargs( 0 ).help( "default:default" );
   params.add_group( "simple" ).title( "Simple group" );
   params.add_parameter( dummy, "--first" ).nargs( 0 ).help( "simple:first" );
   params.add_parameter( dummy, "--second" ).nargs( 0 ).help( "simple:second" );
   params.add_exclusive_group( "exclusive" ).title( "Exclusive group" );
   params.add_parameter( dummy, "--third" ).nargs( 0 ).help( "exclusive:third" );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help, KEEPEMPTY );
   bool hasSimple = false;
   bool hasExclusive = false;
   for ( auto line : helpLines ) {
      if ( strHasText( line, "Simple group:" ) )
         hasSimple = true;
      if ( strHasText( line, "Exclusive group:" ) )
         hasExclusive = true;
   }

   EXPECT_TRUE( hasSimple );
   EXPECT_TRUE( hasExclusive );
}

TEST( ArgumentParserHelpTest, shouldOutputGroupDescription )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().description( "Should output group description." );
   params.add_parameter( dummy, "--default" ).nargs( 0 ).help( "default:default" );
   params.add_group( "simple" ).description( "Simple group." );
   params.add_parameter( dummy, "--first" ).nargs( 0 ).help( "simple:first" );
   params.add_parameter( dummy, "--second" ).nargs( 0 ).help( "simple:second" );
   params.add_exclusive_group( "exclusive" ).description( "Exclusive group." );
   params.add_parameter( dummy, "--third" ).nargs( 0 ).help( "exclusive:third" );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help, KEEPEMPTY );
   bool hasSimple = false;
   bool hasExclusive = false;
   for ( auto line : helpLines ) {
      if ( strHasText( line, "Simple group." ) )
         hasSimple = true;
      if ( strHasText( line, "Exclusive group." ) )
         hasExclusive = true;
   }

   EXPECT_TRUE( hasSimple );
   EXPECT_TRUE( hasExclusive );
}

TEST( ArgumentParserHelpTest, shouldBuildDefaultUsage )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" );
   params.add_parameter( dummy, "--default" ).nargs( 0 );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help, KEEPEMPTY );

   auto posUsage = -1;
   int i = 0;
   for ( auto line : helpLines ) {
      if ( strHasTexts( line, { "usage:", "testing", "--default" } ) )
         posUsage = i;
      ++i;
   }

   EXPECT_LT( -1, posUsage );
}

TEST( ArgumentParserHelpTest, shouldPutOptionsBeforePositionalInUsage )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" );
   params.add_parameter( dummy, "positional" ).nargs( 1 );
   params.add_parameter( dummy, "--option" ).nargs( 0 );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help, KEEPEMPTY );

   auto posUsage = -1;
   int i = 0;
   for ( auto line : helpLines ) {
      if ( strHasTexts( line, { "usage:", "testing", "--option", "positional" } ) )
         posUsage = i;
      ++i;
   }

   EXPECT_LT( -1, posUsage );
}

TEST( ArgumentParserHelpTest, shouldDisplayArgumentCountInUsage )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" );
   params.add_parameter( dummy, "p" ).nargs( 1 );
   params.add_parameter( dummy, "-o" ).nargs( 0 );
   params.add_parameter( dummy, "-i" ).minargs( 1 );
   params.add_parameter( dummy, "-a" ).maxargs( 2 );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help, KEEPEMPTY );

   auto posUsage = -1;
   int i = 0;
   for ( auto line : helpLines ) {
      if ( strHasTexts(
                 line, { "usage:", "testing", "-o", "-i I [I ...]", "-a [A {0..2}]", "p" } ) )
         posUsage = i;
      ++i;
   }

   EXPECT_LT( -1, posUsage );
}

TEST( ArgumentParserHelpTest, shouldDistinguishRequierdOptionsInUsage )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" );
   params.add_parameter( dummy, "-o" ).nargs( 0 ).required( true );
   params.add_parameter( dummy, "-a" ).maxargs( 2 ).required( false );
   params.add_parameter( dummy, "-n" ).nargs( 0 ).required( false );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help, KEEPEMPTY );

   auto posUsage = -1;
   int i = 0;
   for ( auto line : helpLines ) {
      if ( strHasTexts( line, { "usage:", "testing", "-o", "[-a [A {0..2}]]", "[-n]" } ) )
         posUsage = i;
      ++i;
   }

   EXPECT_LT( -1, posUsage );
}

// - argparse/py forbids the use of required keyword with positionals,
//   but positionals can be made optional with nargs='?'
// - we allow required(false) for now
TEST( ArgumentParserHelpTest, shouldDistinguishRequriedPositionalsInUsage )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" );
   params.add_parameter( dummy, "r" ).nargs( 1 ).required( true );
   params.add_parameter( dummy, "o" ).nargs( 1 ).required( false );
   params.add_parameter( dummy, "x" ).maxargs( 1 ).required( false );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help, KEEPEMPTY );

   auto posUsage = -1;
   auto posBad = -1;
   int i = 0;
   for ( auto line : helpLines ) {
      if ( strHasTexts( line, { "usage:", "testing", "r", "[o]", "[x]" } ) )
         posUsage = i;
      if ( strHasTexts( line, { "usage:", "testing", "[[x]]" } ) )
         posBad = i;
      ++i;
   }

   EXPECT_LT( -1, posUsage );
   EXPECT_EQ( -1, posBad );
}

TEST( ArgumentParserHelpTest, shouldUseSamePositionalMetavarNameInUsageAndHelp )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" );
   params.add_parameter( dummy, "xpos" ).nargs( 1 ).required( true );
   params.add_parameter( dummy, "xmetapos" ).metavar( "MPOS" ).nargs( 1 ).required( true );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help, KEEPEMPTY );

   std::map<std::string, long> count{ { "xpos", 0 }, { "XPOS", 0 }, { "xmetapos", 0 },
      { "XMETAPOS", 0 }, { "mpos", 0 }, { "MPOS", 0 } };
   for ( auto line : helpLines ) {
      for ( const auto& tag : count ) {
         if ( strHasText( line, tag.first ) )
            ++count[tag.first];
      }
   }

   EXPECT_EQ( 2, count["xpos"] );
   EXPECT_EQ( 0, count["XPOS"] );
   EXPECT_EQ( 0, count["xmetapos"] );
   EXPECT_EQ( 0, count["XMETAPOS"] );
   EXPECT_EQ( 0, count["mpos"] );
   EXPECT_EQ( 2, count["MPOS"] );
}
