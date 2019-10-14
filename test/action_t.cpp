// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "../src/argparser.h"

#include <gtest/gtest.h>

using namespace argparse;

TEST( ArgumentParserActionTest, shouldSetValueOnTargetWithAction )
{
   auto testAction = []( std::string& target, const std::string& value ) {
      if ( value.find( "1" ) != std::string::npos )
         target = value + " Has One";
      else
         target = value;
   };

   std::string result;
   auto parser = argument_parser{};
   parser.add_argument( result, "-v" ).maxargs( 1 ).action( testAction );

   auto res = parser.parse_args( { "-v", "31" } );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( "31 Has One", result );

   res = parser.parse_args( { "-v", "2" } );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( "2", result );
}

namespace {
class NewType
{
private:
   std::set<long> value;

public:
   NewType( std::set<long> v = {} )
      : value( v )
   {}
   void setValue( std::set<long> v )
   {
      value = v;
   }
   const std::set<long>& getValue() const
   {
      return value;
   }
};

TEST( ArgumentParserActionTest, shouldSetNewTypesThroughActionWithoutFromStringConversion )
{
   auto testAction = []( NewType& target, const std::string& value ) {
      using parser = argparse::argument_parser;
      target = std::set<long>{ (long)value[0], (long)value.size() };
   };

   NewType result;
   auto parser = argument_parser{};
   parser.add_argument( result, "-v" ).maxargs( 1 ).action( testAction );

   auto res = parser.parse_args( { "-v", "assign" } );
   EXPECT_TRUE( res.errors.empty() );

   EXPECT_EQ( 2, result.getValue().size() );
   EXPECT_EQ( 1, result.getValue().count( 'a' ) );
   EXPECT_EQ( 1, result.getValue().count( 6 ) );
}

TEST( ArgumentParserActionTest, shouldSetOptionalNewTypesThroughActionWithoutFromStringConversion )
{
   auto testAction = []( std::optional<NewType>& target, const std::string& value ) {
      using parser = argparse::argument_parser;
      target = std::set<long>{ (long)value[0], (long)value.size() };
   };

   std::optional<NewType> result;
   auto parser = argument_parser{};
   parser.add_argument( result, "-v" ).maxargs( 1 ).action( testAction );

   auto res = parser.parse_args( { "-v", "assign" } );
   EXPECT_TRUE( res.errors.empty() );

   ASSERT_TRUE( static_cast<bool>( result ) );
   EXPECT_EQ( 2, result->getValue().size() );
   EXPECT_EQ( 1, result->getValue().count( 'a' ) );
   EXPECT_EQ( 1, result->getValue().count( 6 ) );
}
}   // namespace
