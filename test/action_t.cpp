// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "../src/argparser.h"

#include <gtest/gtest.h>

using namespace argparse;

TEST( ArgumentParserActionTest, shouldModifyArgumentWithAction )
{
   auto testAction = []( argparse::argument_parser::Value& target,
                           const std::string& value ) -> std::string {
      if ( value.find( "1" ) != std::string::npos )
         return value + " Has One";
      return value;
   };

   std::string result;
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.add_argument( result, "-v" ).nargs( 1 ).action( testAction );

   auto res = parser.parse_args( { "-v", "1" } );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( "1 Has One", result );

   res = parser.parse_args( { "-v", "2" } );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( "2", result );

   res = parser.parse_args( { "-v", "212" } );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( "212 Has One", result );
}

TEST( ArgumentParserActionTest, shouldNotSetValueIfActionReturnsEmptyOptional )
{
   auto testAction = []( argparse::argument_parser::Value& target,
                           const std::string& value ) -> std::string { return {}; };

   std::string result;
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.add_argument( result, "-v" ).maxargs( 1 ).action( testAction );

   auto res = parser.parse_args( { "-v", "1" } );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_TRUE( result.empty() );
}

TEST( ArgumentParserActionTest, shouldSetValueOnTargetFromAction )
{
   auto testAction = []( argparse::argument_parser::Value& target,
                           const std::string& value ) -> std::string {
      if ( value.find( "1" ) != std::string::npos )
         target.setValue( value + " Has One" );
      else
         target.setValue( value );
      return {};
   };

   std::string result;
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.add_argument( result, "-v" ).maxargs( 1 ).action( testAction );

   auto res = parser.parse_args( { "-v", "31" } );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( "31 Has One", result );

   res = parser.parse_args( { "-v", "2" } );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( "2", result );
}
