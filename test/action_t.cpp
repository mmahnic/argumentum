// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <argumentum/argparse.h>

#include <gtest/gtest.h>

using namespace argumentum;

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
   auto params = parser.params();
   params.add_parameter( result, "-v" ).maxargs( 1 ).action( testAction );

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
   auto params = parser.params();
   params.add_parameter( result, "-v" ).maxargs( 1 ).action( testAction );

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
   auto params = parser.params();
   params.add_parameter( result, "-v" ).maxargs( 1 ).action( testAction );

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
   auto params = parser.params();
   params.add_parameter( result, "-v" ).maxargs( 2 ).action( testAction );

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
   auto params = parser.params();
   params.add_parameter( result, "-v" ).maxargs( 1 ).action( testAction );

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
   auto params = parser.params();
   params.add_parameter( result, "-n" ).maxargs( 1 ).action( actionNormal );
   params.add_parameter( result, "-r" ).maxargs( 1 ).action( actionReversed );

   auto res = parser.parse_args( { "-n", "assign", "-r", "vector" } );
   EXPECT_TRUE( res.errors.empty() );

   ASSERT_EQ( 2, result.size() );
   EXPECT_EQ( "assign", result.front() );
   EXPECT_EQ( "rotcev", result.back() );
}

TEST( ArgumentParserActionTest, shouldTerminateParserThroughEnvironmentInAction )
{
   auto actionNormal = []( std::string& target, const std::string& value ) {
      target = value;
   };
   auto actionEnv = []( std::string& target, const std::string& value, Environment& env ) {
      target = value;
      env.exit_parser();
   };

   std::string result;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( result, "-n" ).maxargs( 1 ).action( actionNormal );
   params.add_parameter( result, "-r" ).maxargs( 1 ).action( actionEnv );

   auto res = parser.parse_args( { "-n", "normal", "-r", "environment" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_FALSE( res.errors.empty() );
   EXPECT_TRUE( res.has_exited() );
}

TEST( ArgumentParserActionTest, shouldThrowWhenExitRequestIsUnchecked )
{
   auto actionEnv = []( std::string& target, const std::string& value, Environment& env ) {
      target = value;
      env.exit_parser();
   };

   std::string result;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( result, "-x" ).maxargs( 1 ).action( actionEnv );

   bool caught = false;
   try {
      auto res = parser.parse_args( { "-x" } );
   }
   catch ( const argumentum::UncheckedParseResult& ) {
      caught = true;
   }
   EXPECT_TRUE( caught );
}

TEST( ArgumentParserActionTest, shouldReadOptionNameFromActionEvnironment )
{
   auto actionEnv = []( std::string& target, const std::string& value, Environment& env ) {
      target = value + env.get_option_name();
   };

   std::string result;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( result, "--hide" ).maxargs( 1 ).action( actionEnv );

   auto res = parser.parse_args( { "--hide", "hidden-secret" } );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_FALSE( res.has_exited() );
   EXPECT_EQ( "hidden-secret--hide", result );
}

TEST( ArgumentParserActionTest, shouldReportErrorsThroughActionEvnironment )
{
   auto actionEnv = []( std::string& target, const std::string& value, Environment& env ) {
      env.add_error( "Something is wrong" );
   };

   std::string result;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( result, "--wrong" ).maxargs( 1 ).action( actionEnv );

   auto res = parser.parse_args( { "--wrong", "wrong" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_FALSE( res.has_exited() );
   EXPECT_EQ( "", result );

   ASSERT_FALSE( res.errors.empty() );
   EXPECT_EQ( ACTION_ERROR, res.errors[0].errorCode );
   EXPECT_NE( std::string::npos, res.errors[0].option.find( "--wrong" ) );
   EXPECT_NE( std::string::npos, res.errors[0].option.find( "Something is wrong" ) );
}
