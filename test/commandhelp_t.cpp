// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "helputil.h"

#include "../src/argparser.h"
#include "../src/helpformatter.h"

#include <algorithm>
#include <gtest/gtest.h>
#include <sstream>

using namespace argparse;
using namespace helputil;

namespace {
struct CmdOneOptions : public argparse::CommandOptions
{
   std::optional<std::string> str;
   std::optional<long> count;

   using CommandOptions::CommandOptions;

   void add_arguments( argument_parser& parser ) override
   {
      parser.add_argument( str, "-s" ).nargs( 1 );
      parser.add_argument( count, "-n" ).nargs( 1 );
   }
};

struct CmdTwoOptions : public argparse::CommandOptions
{
   std::optional<std::string> str;
   std::optional<long> count;

   using CommandOptions::CommandOptions;

   void add_arguments( argument_parser& parser ) override
   {
      parser.add_argument( str, "--string" ).nargs( 1 );
      parser.add_argument( count, "--count" ).nargs( 1 );
   }
};

struct GlobalOptions : public argparse::Options
{
   std::optional<std::string> global;
   void add_arguments( argument_parser& parser ) override
   {
      parser.add_argument( global, "str" ).nargs( 1 ).required( true );
   }
};

struct TestCommandOptions : public argparse::Options
{
   std::shared_ptr<GlobalOptions> pGlobal;

   void add_arguments( argument_parser& parser ) override
   {
      auto pGlobal = std::make_shared<GlobalOptions>();
      parser.add_arguments( pGlobal );

      parser.add_command<CmdOneOptions>( "cmdone" ).help( "Command One description." );
      parser.add_command<CmdTwoOptions>( "cmdtwo" ).help( "Command Two description." );
   }
};

}   // namespace

TEST( ArgumentParserCommandHelpTest, shouldOutputCommandSummary )
{
   int dummy;
   auto parser = argument_parser{};

   parser.add_arguments( std::make_shared<TestCommandOptions>() );

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

   parser.add_arguments( std::make_shared<TestCommandOptions>() );

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
   parser.config().program( "testing" );
   parser.add_command<CmdOneOptions>( "one" );

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
