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

TEST( ArgumentParserActionTest, shouldSetVectorNewTypesThroughAction )
{
   auto testAction = []( std::vector<NewType>& target, const std::string& value ) {
      target.push_back( std::set<long>{ (long)value[0], (long)value.size() } );
   };

   std::vector<NewType> result;
   auto parser = argument_parser{};
   parser.add_argument( result, "-v" ).maxargs( 2 ).action( testAction );

   auto res = parser.parse_args( { "-v", "assign", "vector" } );
   EXPECT_TRUE( res.errors.empty() );

   ASSERT_EQ( 2, result.size() );
   auto item = result.front();
   EXPECT_EQ( 2, item.getValue().size() );
   EXPECT_EQ( 1, item.getValue().count( 'a' ) );
   EXPECT_EQ( 1, item.getValue().count( 6 ) );

   item = result.back();
   EXPECT_EQ( 2, item.getValue().size() );
   EXPECT_EQ( 1, item.getValue().count( 'v' ) );
   EXPECT_EQ( 1, item.getValue().count( 6 ) );
}

TEST( ArgumentParserActionTest, shouldSetVectorOptionalNewTypesThroughAction )
{
   auto testAction = []( std::vector<std::optional<NewType>>& target, const std::string& value ) {
      target.push_back( std::set<long>{ (long)value[0], (long)value.size() } );
   };

   std::vector<std::optional<NewType>> result;
   auto parser = argument_parser{};
   parser.add_argument( result, "-v" ).maxargs( 1 ).action( testAction );

   auto res = parser.parse_args( { "-v", "assign" } );
   EXPECT_TRUE( res.errors.empty() );

   ASSERT_EQ( 1, result.size() );
   auto& item = result.front();
   ASSERT_TRUE( static_cast<bool>( item ) );
   EXPECT_EQ( 2, item->getValue().size() );
   EXPECT_EQ( 1, item->getValue().count( 'a' ) );
   EXPECT_EQ( 1, item->getValue().count( 6 ) );
}

}   // namespace

TEST( ArgumentParserActionTest, shouldSetSameVariableThroughMultipleActions )
{
   auto actionNormal = []( std::vector<std::string>& target, const std::string& value ) {
      target.push_back( value );
   };

   auto actionReversed = []( std::vector<std::string>& target, const std::string& value ) {
      auto v = value;
      std::reverse( std::begin( v ), std::end( v ) );
      target.push_back( v );
   };

   std::vector<std::string> result;
   auto parser = argument_parser{};
   parser.add_argument( result, "-n" ).maxargs( 1 ).action( actionNormal );
   parser.add_argument( result, "-r" ).maxargs( 1 ).action( actionReversed );

   auto res = parser.parse_args( { "-n", "assign", "-r", "vector" } );
   EXPECT_TRUE( res.errors.empty() );

   ASSERT_EQ( 2, result.size() );
   EXPECT_EQ( "assign", result.front() );
   EXPECT_EQ( "rotcev", result.back() );
}
