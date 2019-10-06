// Copyright (c) 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "../src/argparser.h"

#include <algorithm>
#include <gtest/gtest.h>

using namespace argparse;

TEST( ArgumentParserCommandTest, shouldHandleCommandsWithSubparsers )
{
   struct CmdOneOptions : public argparse::Options
   {
      std::optional<std::string> str;
      std::optional<long> count;

      void add_arguments( argument_parser& parser ) override
      {
         parser.add_argument( str, "-s" ).nargs( 1 );
         parser.add_argument( count, "-n" ).nargs( 1 );
      }
   };

   struct CmdTwoOptions : public argparse::Options
   {
      std::optional<std::string> str;
      std::optional<long> count;

      void add_arguments( argument_parser& parser ) override
      {
         parser.add_argument( str, "--string" ).nargs( 1 );
         parser.add_argument( count, "--count" ).nargs( 1 );
      }
   };

   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout ).on_exit_return();

   std::shared_ptr<CmdOneOptions> pCmdOne;
   std::shared_ptr<CmdTwoOptions> pCmdTwo;
   parser.add_command( "one", [&]() {
      pCmdOne = std::make_shared<CmdOneOptions>();
      return pCmdOne;
   } );
   parser.add_command( "two", [&]() {
      pCmdTwo = std::make_shared<CmdTwoOptions>();
      return pCmdTwo;
   } );

   // -- WHEN
   auto res = parser.parse_args( { "one", "-s", "works" } );

   // -- THEN
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( nullptr, pCmdTwo );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_TRUE( pCmdOne->str.has_value() );
   EXPECT_EQ( "works", pCmdOne->str.value_or( "" ) );
   EXPECT_FALSE( pCmdOne->count.has_value() );

   // Parsers with commands are not restartable.
   pCmdOne = nullptr;
   pCmdTwo = nullptr;

   // -- WHEN
   res = parser.parse_args( { "two", "--string", "works" } );

   // -- THEN
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( nullptr, pCmdOne );
   ASSERT_NE( nullptr, pCmdTwo );
   EXPECT_TRUE( pCmdTwo->str.has_value() );
   EXPECT_EQ( "works", pCmdTwo->str.value_or( "" ) );
   EXPECT_FALSE( pCmdTwo->count.has_value() );

   pCmdOne = nullptr;
   pCmdTwo = nullptr;

   // -- WHEN
   res = parser.parse_args( { "-s", "works" } );

   // -- THEN
   ASSERT_FALSE( res.errors.empty() );
   EXPECT_EQ( argument_parser::UNKNOWN_OPTION, res.errors.front().errorCode );
}
