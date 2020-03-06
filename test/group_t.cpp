// Copyright (c) 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <argumentum/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>

using namespace argumentum;

TEST( ArgumentParserGroupsTest, shouldDefineExclusiveGroups )
{
   std::optional<int> maybeInt;
   std::optional<int> otherInt;

   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().cout( strout );
   params.add_exclusive_group( "maybies" );
   params.add_parameter( maybeInt, "--maybe" );
   params.add_parameter( maybeInt, "--possibly" );
   params.add_parameter( maybeInt, "--optional" );
   params.end_group();
   params.add_parameter( otherInt, "--other" );

   // -- WHEN there are more than one options from the exclusive group
   auto res = parser.parse_args( { "--maybe", "--optional" } );

   // -- THEN fail
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( EXCLUSIVE_OPTION, res.errors[0].errorCode );

   // -- WHEN there is only one option from the exclusive group
   res = parser.parse_args( { "--maybe" } );

   // -- THEN succeed
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_EQ( 0, res.errors.size() );

   // -- WHEN there are no options from the exclusive group
   res = parser.parse_args( { "--other" } );

   // -- THEN succeed
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_EQ( 0, res.errors.size() );

   // -- WHEN a non-exclusive option and an exclusive option are present
   res = parser.parse_args( { "--maybe", "--other" } );

   // -- THEN succeed
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_EQ( 0, res.errors.size() );
}

// The logic for this requirement may seem strange.  But this might be useful
// when a group of exclusive options is scattered among multiple
// argumentum::Options instances that are used by a parser.
TEST( ArgumentParserGroupsTest, shouldStartSameGroupMultipleTimes )
{
   std::optional<int> maybeInt;
   std::optional<int> otherInt;

   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().cout( strout );
   params.add_exclusive_group( "maybies" );
   params.add_parameter( maybeInt, "--maybe" );
   params.add_parameter( maybeInt, "--optional" );
   params.end_group();
   params.add_parameter( otherInt, "--other" );
   params.add_exclusive_group( "maybies" );
   params.add_parameter( maybeInt, "--possibly" );
   params.end_group();

   // -- WHEN there are more than one options from the exclusive group
   auto res = parser.parse_args( { "--maybe", "--optional" } );

   // -- THEN fail
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( EXCLUSIVE_OPTION, res.errors[0].errorCode );

   // -- WHEN the second argument is from the second group definition
   res = parser.parse_args( { "--maybe", "--possibly" } );

   // -- THEN fail
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( EXCLUSIVE_OPTION, res.errors[0].errorCode );
}

// These groups affect the grouping of the options in the formatted text.
TEST( ArgumentParserGroupsTest, shouldAddGroupsForHelpFormatting )
{
   int first, second, third;

   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().cout( strout );
   params.add_group( "ints" );
   params.add_parameter( first, "--first" );
   params.add_parameter( second, "--second" );
   params.end_group();
   params.add_parameter( third, "--third" );

   auto res = parser.parse_args( { "--first", "--second", "--third" } );
   EXPECT_EQ( 0, res.errors.size() );
}

TEST( ArgumentParserGroupsTest, shouldNotMixSimpleAndExclusiveGroups )
{
   int first, second, third;

   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().cout( strout );
   params.add_group( "ints" );
   params.add_parameter( first, "--first" );
   params.add_parameter( second, "--second" );
   params.end_group();
   EXPECT_THROW( params.add_exclusive_group( "ints" ), argumentum::MixingGroupTypes );
}

TEST( ArgumentParserGroupsTest, shouldNotMixExclusiveAndSimpleGroups )
{
   int first, second, third;

   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().cout( strout );
   params.add_exclusive_group( "ints" );
   params.add_parameter( first, "--first" );
   params.add_parameter( second, "--second" );
   params.end_group();
   EXPECT_THROW( params.add_group( "ints" ), argumentum::MixingGroupTypes );
}

TEST( ArgumentParserGroupsTest, shouldRequireOptionsFromRequiredExclusiveGroups )
{
   int first, second, third;

   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().cout( strout );
   params.add_exclusive_group( "ints" ).required( true );
   params.add_parameter( first, "--first" );
   params.add_parameter( second, "--second" );
   params.end_group();
   params.add_parameter( third, "--third" );

   auto res = parser.parse_args( { "--third" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( MISSING_OPTION_GROUP, res.errors[0].errorCode );
}

TEST( ArgumentParserGroupsTest, shouldRequireOptionsFromRequiredSimpleGroups )
{
   int first, second, third;

   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().cout( strout );
   params.add_group( "ints" ).required( true );
   params.add_parameter( first, "--first" );
   params.add_parameter( second, "--second" );
   params.end_group();
   params.add_parameter( third, "--third" );

   auto res = parser.parse_args( { "--third" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( MISSING_OPTION_GROUP, res.errors[0].errorCode );
}

TEST( ArgumentParserGroupsTest, shouldForbidRequiredOptionsInExclusiveGroup )
{
   int first, second;

   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().cout( strout );
   params.add_exclusive_group( "ints" );
   params.add_parameter( first, "--first" );
   params.add_parameter( second, "--second" ).required( true );

   // The combination of exclusive groups with required options can be verified
   // only after the options are configured.  We make such checks in parse_args
   // before actually parsing anything.
   EXPECT_THROW( parser.parse_args( {} ), argumentum::RequiredExclusiveOption );
}

TEST( ArgumentParserGroupsTest, shouldNotAddDefaultHelpToGroupWhenHelpNotDefined )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().cout( strout );
   params.add_group( "simple" );
   try {
      parser.parse_args( {} );
   }
   catch ( ... ) {
   }

   auto args = parser.describe_arguments();
   std::optional<bool> shortInGroup;
   std::optional<bool> longInGroup;
   for ( auto& arg : args ) {
      if ( arg.short_name == "-h" )
         shortInGroup = !arg.group.name.empty();
      if ( arg.long_name == "--help" )
         longInGroup = !arg.group.name.empty();
   }

   EXPECT_TRUE( shortInGroup.has_value() );
   EXPECT_FALSE( shortInGroup.value_or( true ) );
   EXPECT_TRUE( longInGroup.has_value() );
   EXPECT_FALSE( longInGroup.value_or( true ) );
}
