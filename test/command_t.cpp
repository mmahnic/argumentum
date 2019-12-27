// Copyright (c) 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "testutil.h"

#include <argumentum/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>

using namespace argumentum;
using namespace testutil;

namespace {
struct CmdOneOptions : public argumentum::CommandOptions
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

struct CmdTwoOptions : public argumentum::CommandOptions
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
}   // namespace

TEST( ArgumentParserCommandTest, shouldHandleCommandsWithSubparsers )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   parser.add_command<CmdOneOptions>( "one" );
   parser.add_command<CmdTwoOptions>( "two" );

   // -- WHEN
   auto res = parser.parse_args( { "one", "-s", "works" } );

   // -- THEN
   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   auto pCmdTwo = findCommand<CmdTwoOptions>( res, "two" );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( nullptr, pCmdTwo );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_TRUE( pCmdOne->str.has_value() );
   EXPECT_EQ( "works", pCmdOne->str.value_or( "" ) );
   EXPECT_FALSE( pCmdOne->count.has_value() );

   // -- WHEN
   res = parser.parse_args( { "two", "--string", "works" } );

   // -- THEN
   pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   pCmdTwo = findCommand<CmdTwoOptions>( res, "two" );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( nullptr, pCmdOne );
   ASSERT_NE( nullptr, pCmdTwo );
   EXPECT_TRUE( pCmdTwo->str.has_value() );
   EXPECT_EQ( "works", pCmdTwo->str.value_or( "" ) );
   EXPECT_FALSE( pCmdTwo->count.has_value() );

   // -- WHEN
   res = parser.parse_args( { "-s", "works" } );

   // -- THEN
   pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   pCmdTwo = findCommand<CmdTwoOptions>( res, "two" );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_FALSE( res.errors.empty() );
   EXPECT_EQ( UNKNOWN_OPTION, res.errors.front().errorCode );
   EXPECT_EQ( nullptr, pCmdOne );
   EXPECT_EQ( nullptr, pCmdTwo );
}

// Form: program --global-options command --command-options
TEST( ArgumentParserCommandTest, shouldHandleGlobalOptionsWhenCommandsPresent )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   std::optional<std::string> global;
   parser.add_argument( global, "-s" ).nargs( 1 );
   parser.add_command<CmdOneOptions>( "one" );

   auto res = parser.parse_args( { "-s", "global-works", "one", "-s", "command-works" } );
   EXPECT_TRUE( res.errors.empty() );

   EXPECT_TRUE( global.has_value() );
   EXPECT_EQ( "global-works", global.value_or( "" ) );

   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_EQ( "command-works", pCmdOne->str.value_or( "" ) );
}

// A rewrite of the previous test with GlobalOptions structure.
TEST( ArgumentParserCommandTest, shouldHandleGlobalOptionsWhenCommandsPresent2 )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   struct GlobalOptions : public argumentum::Options
   {
      std::optional<std::string> global;
      void add_arguments( argument_parser& parser ) override
      {
         parser.add_argument( global, "-s" ).nargs( 1 );
      }
   };

   auto pGlobal = std::make_shared<GlobalOptions>();
   ASSERT_NE( nullptr, pGlobal );
   parser.add_arguments( pGlobal );
   parser.add_command<CmdOneOptions>( "one" );

   auto res = parser.parse_args( { "-s", "global-works", "one", "-s", "command-works" } );
   EXPECT_TRUE( res.errors.empty() );

   EXPECT_TRUE( pGlobal->global.has_value() );
   EXPECT_EQ( "global-works", pGlobal->global.value_or( "" ) );

   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_EQ( "command-works", pCmdOne->str.value_or( "" ) );
}

TEST( ArgumentParserCommandTest, shouldRequireParentsRequiredOptionsWhenCommandPresent )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   struct GlobalOptions : public argumentum::Options
   {
      std::optional<std::string> global;
      void add_arguments( argument_parser& parser ) override
      {
         parser.add_argument( global, "-s" ).nargs( 1 ).required( true );
      }
   };

   auto pGlobal = std::make_shared<GlobalOptions>();
   ASSERT_NE( nullptr, pGlobal );
   parser.add_arguments( pGlobal );
   parser.add_command<CmdOneOptions>( "one" );

   auto res = parser.parse_args( { "one", "-s", "command-works" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_FALSE( res.errors.empty() );
   EXPECT_EQ( MISSING_OPTION, res.errors.front().errorCode );

   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_EQ( "command-works", pCmdOne->str.value_or( "" ) );
}

// The parser tries to process commands before positional arguments. If an
// allowed argument value is equal to the name of a command, the ambiguity is
// resolved in favour of the command.
TEST( ArgumentParserCommandTest, shouldRequireParentsRequiredPositionalWhenCommandPresent )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   struct GlobalOptions : public argumentum::Options
   {
      std::optional<std::string> global;
      void add_arguments( argument_parser& parser ) override
      {
         parser.add_argument( global, "str" ).nargs( 1 ).required( true );
      }
   };

   auto pGlobal = std::make_shared<GlobalOptions>();
   ASSERT_NE( nullptr, pGlobal );
   parser.add_arguments( pGlobal );
   parser.add_command<CmdOneOptions>( "one" );

   auto res = parser.parse_args( { "one", "-s", "command-works" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_FALSE( res.errors.empty() );
   EXPECT_EQ( MISSING_ARGUMENT, res.errors.front().errorCode );

   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_EQ( "command-works", pCmdOne->str.value_or( "" ) );
}

TEST( ArgumentParserCommandTest, shouldStoreInstantiatedCommandsInParseResults )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   parser.add_command<CmdOneOptions>( "one" );
   parser.add_command<CmdTwoOptions>( "two" );

   // -- WHEN
   auto res = parser.parse_args( { "one", "-s", "works" } );

   // -- THEN
   EXPECT_TRUE( res.errors.empty() );
   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   auto pCmdTwo = findCommand<CmdTwoOptions>( res, "two" );
   EXPECT_EQ( nullptr, pCmdTwo );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_TRUE( pCmdOne->str.has_value() );
   EXPECT_EQ( "works", pCmdOne->str.value_or( "" ) );
   EXPECT_FALSE( pCmdOne->count.has_value() );

   // -- WHEN
   res = parser.parse_args( { "two", "--string", "works" } );

   // -- THEN
   EXPECT_TRUE( res.errors.empty() );
   pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   pCmdTwo = findCommand<CmdTwoOptions>( res, "two" );
   EXPECT_EQ( nullptr, pCmdOne );
   ASSERT_NE( nullptr, pCmdTwo );
   EXPECT_TRUE( pCmdTwo->str.has_value() );
   EXPECT_EQ( "works", pCmdTwo->str.value_or( "" ) );
   EXPECT_FALSE( pCmdTwo->count.has_value() );
}

TEST( ArgumentParserCommandTest, shouldReportErrorsOnlyInTopLevelParser )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   parser.add_command<CmdOneOptions>( "one" );

   // -- WHEN
   auto res = parser.parse_args( { "one", "--bad-option" } );
   EXPECT_FALSE( static_cast<bool>( res ) );

   // -- THEN
   auto help = strout.str();
   auto lines = splitLines( help );
   int count = 0;
   for ( auto line : lines ) {
      if ( strHasText( line, "--bad-option" ) )
         ++count;
   }
   EXPECT_EQ( 1, count );
}
