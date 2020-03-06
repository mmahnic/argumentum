// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.
//
// This file tests deprecated interfaces to make sure they still work until they
// are removed.

#include "testutil.h"
#include "vectors.h"

#include <argumentum/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>

using namespace argumentum;
using namespace testing;
using namespace testutil;

namespace {
struct OptionPack : public argumentum::Options
{
   std::optional<std::string> str;

   void add_arguments( argument_parser& parser ) override
   {
      parser.add_argument( str, "-s" ).nargs( 1 );
   }
};

struct CmdOneOptions : public argumentum::CommandOptions
{
   std::optional<std::string> str;

   using CommandOptions::CommandOptions;

   void add_arguments( argument_parser& parser ) override
   {
      parser.add_argument( str, "-s" ).nargs( 1 );
   }
};
}   // namespace

TEST( ParserArguments, shouldAddInstantiatedCommands )
{
   auto parser = argument_parser{};
   auto pCmdOne = std::make_shared<CmdOneOptions>( "one" );

   // -- WHEN
   parser.add_command( pCmdOne );

   // -- THEN
   auto res = parser.parse_args( { "one", "-s", "works" } );
   EXPECT_TRUE( res );
   EXPECT_EQ( "works", pCmdOne->str.value_or( "" ) );
}

TEST( ParserArguments, shouldAddOptions )
{
   auto parser = argument_parser{};
   auto pOptions = std::make_shared<OptionPack>();

   // -- WHEN
   parser.add_arguments( pOptions );

   // -- THEN
   auto res = parser.parse_args( { "-s", "works" } );
   EXPECT_TRUE( res );
   EXPECT_EQ( "works", pOptions->str.value_or( "" ) );
}

TEST( ParserArguments, shouldAddArguments )
{
   auto parser = argument_parser{};

   // -- WHEN
   std::string str;
   std::optional<std::string> optStr;
   parser.add_argument( str, "-s" ).nargs( 1 );
   parser.add_argument( optStr, "-o" ).nargs( 1 );

   // -- THEN
   auto res = parser.parse_args( { "-s", "works", "-o", "works" } );
   EXPECT_TRUE( res );
   EXPECT_EQ( "works", str );
   EXPECT_EQ( "works", optStr.value_or( "" ) );
}

TEST( ParserArguments, shouldAddDefaultHelpOption )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   // -- WHEN
   parser.add_default_help_option();

   // -- THEN
   auto res = parser.parse_args( { "-h" } );
   EXPECT_FALSE( res );
   EXPECT_TRUE( res.help_was_shown() );
}

TEST( ParserArguments, shouldAddCustomHelpOption )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   // -- WHEN
   parser.add_help_option( "-a" );

   // -- THEN
   auto res = parser.parse_args( { "-a" } );
   EXPECT_FALSE( res );
   EXPECT_TRUE( res.help_was_shown() );
}

TEST( ParserArguments, shouldAddGroup )
{
   auto parser = argument_parser{};
   std::string str;

   // -- WHEN
   parser.add_group( "testgroup" );
   parser.add_argument( str, "-s" ).nargs( 1 );
   parser.add_argument( str, "-t" ).nargs( 1 );
   parser.end_group();

   // -- THEN
   auto res = parser.parse_args( { "-s", "works", "-t", "works" } );
   EXPECT_TRUE( res );
   EXPECT_EQ( "works", str );
}

TEST( ParserArguments, shouldAddExclusiveGroup )
{
   auto parser = argument_parser{};
   std::string str;

   // -- WHEN
   parser.add_exclusive_group( "testgroup" );
   parser.add_argument( str, "-s" ).nargs( 1 );
   parser.add_argument( str, "-t" ).nargs( 1 );
   parser.end_group();

   // -- THEN
   auto res = parser.parse_args( { "-s", "works", "-t", "fails" } );
   EXPECT_FALSE( res );
}
