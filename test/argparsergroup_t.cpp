// Copyright (c) 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "../src/argparser.h"

#include <algorithm>
#include <gtest/gtest.h>

using namespace argparse;

TEST( ArgumentParserGroupsTest, shouldDefineExclusiveGroups )
{
   std::optional<int> maybeInt;
   std::optional<int> otherInt;

   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout ).on_exit_return();
   parser.add_exclusive_group( "maybies" );
   parser.add_argument( maybeInt, "--maybe" );
   parser.add_argument( maybeInt, "--possibly" );
   parser.add_argument( maybeInt, "--optional" );
   parser.end_group();
   parser.add_argument( otherInt, "--other" );

   // -- WHEN there are more than one options from the exclusive group
   auto res = parser.parse_args( { "--maybe", "--optional" } );

   // -- THEN fail
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::EXCLUSIVE_OPTION, res.errors[0].errorCode );

   // -- WHEN there is only one option from the exclusive group
   res = parser.parse_args( { "--maybe" } );

   // -- THEN succeed
   EXPECT_EQ( 0, res.errors.size() );

   // -- WHEN there are no options from the exclusive group
   res = parser.parse_args( { "--other" } );

   // -- THEN succeed
   EXPECT_EQ( 0, res.errors.size() );

   // -- WHEN a non-exclusive option and an exclusive option are present
   res = parser.parse_args( { "--maybe", "--other" } );

   // -- THEN succeed
   EXPECT_EQ( 0, res.errors.size() );
}

// The logic for this requirement may seem strange.  But this might be useful
// when a group of exclusive options is scattered among multiple
// argparse::Options instances that are used by a parser.
TEST( ArgumentParserGroupsTest, shouldStartSameGroupMultipleTimes )
{
   std::optional<int> maybeInt;
   std::optional<int> otherInt;

   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout ).on_exit_return();
   parser.add_exclusive_group( "maybies" );
   parser.add_argument( maybeInt, "--maybe" );
   parser.add_argument( maybeInt, "--optional" );
   parser.end_group();
   parser.add_argument( otherInt, "--other" );
   parser.add_exclusive_group( "maybies" );
   parser.add_argument( maybeInt, "--possibly" );
   parser.end_group();

   // -- WHEN there are more than one options from the exclusive group
   auto res = parser.parse_args( { "--maybe", "--optional" } );

   // -- THEN fail
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::EXCLUSIVE_OPTION, res.errors[0].errorCode );

   // -- WHEN the second argument is from the second group definition
   res = parser.parse_args( { "--maybe", "--possibly" } );

   // -- THEN fail
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::EXCLUSIVE_OPTION, res.errors[0].errorCode );
}

// These groups affect the grouping of the options in the formatted text.
TEST( ArgumentParserGroupsTest, shouldAddGroupsForHelpFormatting )
{
   int first, second, third;

   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout ).on_exit_return();
   parser.add_group( "ints" );
   parser.add_argument( first, "--first" );
   parser.add_argument( second, "--second" );
   parser.end_group();
   parser.add_argument( third, "--third" );

   auto res = parser.parse_args( { "--first", "--second", "--third" } );
   EXPECT_EQ( 0, res.errors.size() );
}

TEST( ArgumentParserGroupsTest, shouldNotMixSimpleAndExclusiveGroups )
{
   int first, second, third;

   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout ).on_exit_return();
   parser.add_group( "ints" );
   parser.add_argument( first, "--first" );
   parser.add_argument( second, "--second" );
   parser.end_group();
   EXPECT_THROW( parser.add_exclusive_group( "ints" ), argparse::MixingGroupTypes );
}

TEST( ArgumentParserGroupsTest, shouldNotMixExclusiveAndSimpleGroups )
{
   int first, second, third;

   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout ).on_exit_return();
   parser.add_exclusive_group( "ints" );
   parser.add_argument( first, "--first" );
   parser.add_argument( second, "--second" );
   parser.end_group();
   EXPECT_THROW( parser.add_group( "ints" ), argparse::MixingGroupTypes );
}

TEST( ArgumentParserGroupsTest, shouldRequireOptionsFromRequiredExclusiveGroups )
{
   int first, second, third;

   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout ).on_exit_return();
   parser.add_exclusive_group( "ints" ).required( true );
   parser.add_argument( first, "--first" );
   parser.add_argument( second, "--second" );
   parser.end_group();
   parser.add_argument( third, "--third" );

   auto res = parser.parse_args( { "--third" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::MISSING_OPTION_GROUP, res.errors[0].errorCode );
}

TEST( ArgumentParserGroupsTest, shouldRequireOptionsFromRequiredSimpleGroups )
{
   int value, first, second, third;

   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout ).on_exit_return();
   parser.add_group( "ints" ).required( true );
   parser.add_argument( first, "--first" );
   parser.add_argument( second, "--second" );
   parser.end_group();
   parser.add_argument( third, "--third" );

   auto res = parser.parse_args( { "--third" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::MISSING_OPTION_GROUP, res.errors[0].errorCode );
}
