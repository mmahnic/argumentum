// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

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

   void add_parameters( ParameterConfig& params ) override
   {
      params.add_parameter( str, "-s" ).nargs( 1 );
   }
};

struct CmdOneOptions : public argumentum::CommandOptions
{
   std::optional<std::string> str;

   using CommandOptions::CommandOptions;

   void add_parameters( ParameterConfig& params ) override
   {
      params.add_parameter( str, "-s" ).nargs( 1 );
   }
};
}   // namespace

TEST( ArgumentConfig, shouldAddInstantiatedCommands )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   auto pCmdOne = std::make_shared<CmdOneOptions>( "one" );

   // -- WHEN
   params.add_command( pCmdOne );

   // -- THEN
   auto res = parser.parse_args( { "one", "-s", "works" } );
   EXPECT_TRUE( res );
   EXPECT_EQ( "works", pCmdOne->str.value_or( "" ) );
}

TEST( ArgumentConfig, shouldAddOptions )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   auto pOptions = std::make_shared<OptionPack>();

   // -- WHEN
   params.add_parameters( pOptions );

   // -- THEN
   auto res = parser.parse_args( { "-s", "works" } );
   EXPECT_TRUE( res );
   EXPECT_EQ( "works", pOptions->str.value_or( "" ) );
}

TEST( ArgumentConfig, shouldAddArguments )
{
   auto parser = argument_parser{};
   auto params = parser.params();

   // -- WHEN
   std::string str;
   std::optional<std::string> optStr;
   params.add_parameter( str, "-s" ).nargs( 1 );
   params.add_parameter( optStr, "-o" ).nargs( 1 );

   // -- THEN
   auto res = parser.parse_args( { "-s", "works", "-o", "works" } );
   EXPECT_TRUE( res );
   EXPECT_EQ( "works", str );
   EXPECT_EQ( "works", optStr.value_or( "" ) );
}

TEST( ArgumentConfig, shouldAddDefaultHelpOption )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   auto params = parser.params();

   // -- WHEN
   params.add_default_help_option();

   // -- THEN
   auto res = parser.parse_args( { "-h" } );
   EXPECT_FALSE( res );
   EXPECT_TRUE( res.help_was_shown() );
}

TEST( ArgumentConfig, shouldAddCustomHelpOption )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   auto params = parser.params();

   // -- WHEN
   params.add_help_option( "-a" );

   // -- THEN
   auto res = parser.parse_args( { "-a" } );
   EXPECT_FALSE( res );
   EXPECT_TRUE( res.help_was_shown() );
}

TEST( ArgumentConfig, shouldAddGroup )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   std::string str;

   // -- WHEN
   params.add_group( "testgroup" );
   params.add_parameter( str, "-s" ).nargs( 1 );
   params.add_parameter( str, "-t" ).nargs( 1 );
   params.end_group();

   // -- THEN
   auto res = parser.parse_args( { "-s", "works", "-t", "works" } );
   EXPECT_TRUE( res );
   EXPECT_EQ( "works", str );
}

TEST( ArgumentConfig, shouldAddExclusiveGroup )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   std::string str;

   // -- WHEN
   params.add_exclusive_group( "testgroup" );
   params.add_parameter( str, "-s" ).nargs( 1 );
   params.add_parameter( str, "-t" ).nargs( 1 );
   params.end_group();

   // -- THEN
   auto res = parser.parse_args( { "-s", "works", "-t", "fails" } );
   EXPECT_FALSE( res );
}
