// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "testutil.h"

#include <argumentum/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>
#include <sstream>

using namespace argumentum;
using namespace testutil;

namespace {
struct CmdOneOptions : public argumentum::CommandOptions
{
   std::optional<std::string> str;
   std::optional<long> count;

   using CommandOptions::CommandOptions;

   void add_parameters( ParameterConfig& params ) override
   {
      params.add_parameter( str, "-s" ).nargs( 1 );
      params.add_parameter( count, "-n" ).nargs( 1 );
   }
};

struct CmdTwoOptions : public argumentum::CommandOptions
{
   std::optional<std::string> str;
   std::optional<long> count;

   using CommandOptions::CommandOptions;

   void add_parameters( ParameterConfig& params ) override
   {
      params.add_parameter( str, "--string" ).nargs( 1 );
      params.add_parameter( count, "--count" ).nargs( 1 );
   }
};

struct GlobalOptions : public argumentum::Options
{
   std::optional<std::string> global;
   void add_parameters( ParameterConfig& params ) override
   {
      params.add_parameter( global, "str" ).nargs( 1 ).required( true );
   }
};

struct TestCommandOptions : public argumentum::Options
{
   std::shared_ptr<GlobalOptions> pGlobal;

   void add_parameters( ParameterConfig& params ) override
   {
      auto pGlobal = std::make_shared<GlobalOptions>();
      params.add_parameters( pGlobal );

      params.add_command<CmdOneOptions>( "cmdone" ).help( "Command One description." );
      params.add_command<CmdTwoOptions>( "cmdtwo" ).help( "Command Two description." );
   }
};

}   // namespace

TEST( ArgumentParserCommandHelpTest, shouldOutputCommandSummary )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();

   params.add_parameters( std::make_shared<TestCommandOptions>() );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help, KEEPEMPTY );
   bool hasOne = false;
   bool hasTwo = false;
   for ( auto line : helpLines ) {
      if ( strHasTexts( line, { "cmdone", "Command One description." } ) )
         hasOne = true;
      if ( strHasTexts( line, { "cmdtwo", "Command Two description." } ) )
         hasTwo = true;
   }

   EXPECT_TRUE( hasOne );
   EXPECT_TRUE( hasTwo );
}

TEST( ArgumentParserCommandHelpTest, shouldPutUngroupedCommandsUnderCommandsTitle )
{
   int dummy;
   auto parser = argument_parser{};
   auto params = parser.params();

   params.add_parameters( std::make_shared<TestCommandOptions>() );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help, KEEPEMPTY );
   int posPositional = -1;
   int posOne = -1;
   int posTwo = -1;
   int posTitle = -1;

   int i = 0;
   for ( auto line : helpLines ) {
      if ( strHasTexts( line, { "cmdone", "Command One description." } ) )
         posOne = i;
      if ( strHasTexts( line, { "cmdtwo", "Command Two description." } ) )
         posTwo = i;
      if ( strHasText( line, "commands:" ) )
         posTitle = i;
      if ( strHasText( line, "positional arguments:" ) )
         posPositional = i;
      ++i;
   }

   EXPECT_LT( -1, posPositional );
   EXPECT_LT( -1, posOne );
   EXPECT_LT( -1, posTwo );
   EXPECT_LT( -1, posTitle );

   EXPECT_LT( posPositional, posTitle );
   EXPECT_LT( posTitle, posOne );
   EXPECT_LT( posTitle, posTwo );
}

TEST( ArgumentParserCommandHelpTest, shouldShowCommandPlaceholderInUsage )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" );
   params.add_command<CmdOneOptions>( "one" );

   auto help = getTestHelp( parser, HelpFormatter() );
   auto helpLines = splitLines( help, KEEPEMPTY );

   auto posUsage = -1;
   auto posOne = -1;
   auto posS = -1;
   int i = 0;
   for ( auto line : helpLines ) {
      if ( strHasTexts( line, { "usage:", "testing", "<command> ..." } ) )
         posUsage = i;
      if ( strHasTexts( line, { "usage:", "-s" } ) )
         posS = i;
      if ( strHasTexts( line, { "usage:", "one" } ) )
         posOne = i;
      ++i;
   }

   EXPECT_LT( -1, posUsage );
   EXPECT_EQ( -1, posOne );
   EXPECT_EQ( -1, posS );
}

TEST( ArgumentParserCommandHelpTest, shouldNotDisplayCommandHelpIfCommandNotGiven )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" ).cout( strout );
   params.add_command<CmdOneOptions>( "one" );
   params.add_command<CmdTwoOptions>( "two" );

   auto res = parser.parse_args( { "--help" } );
   EXPECT_FALSE( static_cast<bool>( res ) );

   auto help = strout.str();
   EXPECT_FALSE( strHasText( help, "-s " ) );
   EXPECT_FALSE( strHasText( help, "-n " ) );
   EXPECT_FALSE( strHasText( help, "--string " ) );
   EXPECT_FALSE( strHasText( help, "--count " ) );
}

TEST( ArgumentParserCommandHelpTest, shouldDisplayCommandHelpIfCommandGivenBeforeFlag )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" ).cout( strout );
   params.add_command<CmdOneOptions>( "one" );
   params.add_command<CmdTwoOptions>( "two" );

   // -- WHEN
   auto res = parser.parse_args( { "one", "--help" } );
   EXPECT_FALSE( static_cast<bool>( res ) );

   // -- THEN
   auto help = strout.str();
   EXPECT_TRUE( strHasText( help, "-s " ) );
   EXPECT_TRUE( strHasText( help, "-n " ) );
   EXPECT_FALSE( strHasText( help, "--string " ) );
   EXPECT_FALSE( strHasText( help, "--count " ) );

   // -- WHEN
   strout.str( "" );
   res = parser.parse_args( { "two", "--help" } );
   EXPECT_FALSE( static_cast<bool>( res ) );

   // -- THEN
   help = strout.str();
   EXPECT_FALSE( strHasText( help, "-s " ) );
   EXPECT_FALSE( strHasText( help, "-n " ) );
   EXPECT_TRUE( strHasText( help, "--string " ) );
   EXPECT_TRUE( strHasText( help, "--count " ) );
}

TEST( ArgumentParserCommandHelpTest, shouldDisplayCommandNameInCommandHelp )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" ).cout( strout );
   params.add_command<CmdOneOptions>( "one" );

   // -- WHEN
   auto res = parser.parse_args( { "one", "--help" } );
   EXPECT_FALSE( static_cast<bool>( res ) );

   // -- THEN
   auto help = strout.str();
   auto helpLines = splitLines( help );
   int countUsageName = 0;
   for ( auto& line : helpLines ) {
      if ( strHasTexts( line, { "usage:", "one" } ) )
         ++countUsageName;
   }
   EXPECT_EQ( 1, countUsageName ) << "----\n" << help;
}

TEST( ArgumentParserCommandHelpTest, shouldDisplayCommandDescriptionInCommandHelp )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" ).cout( strout );
   params.add_command<CmdOneOptions>( "one" ).help( "Command One description." );

   // -- WHEN
   auto res = parser.parse_args( { "one", "--help" } );
   EXPECT_FALSE( static_cast<bool>( res ) );

   // -- THEN
   auto help = strout.str();
   auto helpLines = splitLines( help );
   int countDescr = 0;
   for ( auto& line : helpLines ) {
      if ( strHasText( line, "Command One description." ) )
         ++countDescr;
   }
   EXPECT_EQ( 1, countDescr ) << "----\n" << help;
}

TEST( ArgumentParserCommandHelpTest, shouldDisplayCommandHelpForDeepestCommandOnly )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" ).cout( strout ).description( "Tester." );
   params.add_command<CmdOneOptions>( "one" ).help( "Command One description." );

   // -- WHEN
   auto res = parser.parse_args( { "one", "--help" } );
   EXPECT_FALSE( static_cast<bool>( res ) );

   // -- THEN
   auto help = strout.str();
   auto helpLines = splitLines( help );
   int countTesting = 0;
   int countTester = 0;
   int countOne = 0;
   int countDescr = 0;
   for ( auto& line : helpLines ) {
      if ( strHasTexts( line, { "usage:", "testing" } ) )
         ++countTesting;
      if ( strHasText( line, "Tester." ) )
         ++countTester;
      if ( strHasTexts( line, { "usage:", "testing", "one" } ) )
         ++countOne;
      if ( strHasText( line, "Command One description." ) )
         ++countDescr;
   }
   EXPECT_EQ( 1, countTesting );   // From the 'one' command path
   EXPECT_EQ( 0, countTester );
   EXPECT_EQ( 1, countOne );
   EXPECT_EQ( 1, countDescr );
}

#if 0
// This test is disabled.  Command after help option might be processed only in the
// top-level parser.  Subparsers should stop processing at the help
// option. (This is the way it is done in Git.)
// TODO: maybe we could support this feature in all levels.  The help action
// would have to access next option-s.
TEST( ArgumentParserCommandHelpTest, shouldDisplayCommandHelpIfCommandGivenAfterFlag )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( "testing" ).cout( strout );
   params.add_command<CmdOneOptions>( "one" );

   // -- WHEN
   auto res = parser.parse_args( { "--help", "one" } );
   EXPECT_FALSE( static_cast<bool>( res ) );

   // -- THEN
   auto help = strout.str();
   EXPECT_TRUE( strHasText( help, "-s " ) );
   EXPECT_TRUE( strHasText( help, "-n " ) );
}
#endif
