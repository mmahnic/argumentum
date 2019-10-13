// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "../src/argparser.h"

#include <gtest/gtest.h>

using namespace argparse;

#if 0
// TODO: Obsolete test after action redesign
TEST( ArgumentParserActionTest, shouldModifyArgumentWithAction )
{
   auto testAction = []( std::string& target,
                           const std::string& value ) -> std::optional<std::string> {
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

// TODO: Obsolete test after action redesign
TEST( ArgumentParserActionTest, shouldNotSetValueIfActionReturnsEmptyOptional )
{
   auto testAction = []( std::string& target,
                           const std::string& value ) -> std::optional<std::string> { return {}; };

   std::string result;
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.add_argument( result, "-v" ).maxargs( 1 ).action( testAction );

   auto res = parser.parse_args( { "-v", "1" } );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_TRUE( result.empty() );
}
#endif

TEST( ArgumentParserActionTest, shouldSetValueOnTargetFromAction )
{
   auto testAction = []( std::string& target, const std::string& value ) {
      std::cout << "target: '" << target << "', value: '" << value << "'\n";
      if ( value.find( "1" ) != std::string::npos )
         target = value + " Has One";
      else
         target = value;
      std::cout << " --> target: '" << target << "'\n";
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

   // - add_argument fails because there is no conversion from string to NewType.
   // - testAction fails because mValue is not part of target (of type Value, base class).
   parser.add_argument( result, "-v" ).maxargs( 1 ).action( testAction );

   auto res = parser.parse_args( { "-v", "assign" } );
   EXPECT_TRUE( res.errors.empty() );

   EXPECT_EQ( 2, result.getValue().size() );
   EXPECT_EQ( 1, result.getValue().count( 'a' ) );
   EXPECT_EQ( 1, result.getValue().count( 6 ) );
}
}   // namespace
