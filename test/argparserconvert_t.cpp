// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "vectors.h"
#include "../src/argparser.h"

#include <algorithm>
#include <gtest/gtest.h>

using namespace argparse;
using namespace testing;

namespace {
}

TEST( ArgumentParserConvertTest, shouldParseIntegerValues )
{
   long value = 0;

   auto parser = argument_parser{};
   parser.add_argument( value, "-v", "--value" ).nargs( 1 );
   parser.parse_args( { "--value", "2314" } );

   EXPECT_EQ( 2314, value );
}

TEST( ArgumentParserConvertTest, shouldParseOptionalIntegerValues )
{
   std::optional<long> value;
   std::optional<long> missing;

   auto parser = argument_parser{};
   parser.add_argument( value, "-v", "--value" ).nargs( 1 );
   parser.add_argument( missing, "-m", "--missing" ).nargs( 1 );
   parser.parse_args( { "--value", "2314" } );

   EXPECT_FALSE( static_cast<bool>( missing ) );
   EXPECT_TRUE( static_cast<bool>( value ) );
   EXPECT_EQ( 2314, value.value_or( 0 ) );
}

TEST( ArgumentParserConvertTest, shouldSupportRawValueTypes )
{
   std::string strvalue;
   long intvalue = 1;
   double floatvalue = 2.0;

   auto parser = argument_parser{};
   parser.add_argument( strvalue, "--str" ).nargs( 1 );
   parser.add_argument( intvalue, "--int" ).nargs( 1 );
   parser.add_argument( floatvalue, "--float" ).nargs( 1 );

   auto res = parser.parse_args( { "--str", "string", "--int", "2134", "--float", "32.4" } );
   EXPECT_EQ( "string", strvalue );
   EXPECT_EQ( 2134, intvalue );
   EXPECT_NEAR( 32.4, floatvalue, 1e-9 );
}

TEST( ArgumentParserConvertTest, shouldSupportFlagValues )
{
   std::optional<std::string> flag;

   auto parser = argument_parser{};
   parser.add_argument( flag, "-a" ).flagValue( "from-a" );
   parser.add_argument( flag, "-b" ).flagValue( "from-b" );

   auto res = parser.parse_args( { "-a", "-b" } );
   EXPECT_EQ( "from-b", flag.value() );

   flag = {};
   res = parser.parse_args( { "-b", "-a" } );
   EXPECT_EQ( "from-a", flag.value() );
}

TEST( ArgumentParserConvertTest, shouldSupportFloatingPointValues )
{
   std::optional<double> value;

   auto parser = argument_parser{};
   parser.add_argument( value, "-a" ).nargs( 1 );

   auto res = parser.parse_args( { "-a", "23.5" } );
   EXPECT_NEAR( 23.5, value.value(), 1e-9 );
}

TEST( ArgumentParserConvertTest, shouldReportBadConversionError )
{
   std::optional<long> flagA;

   auto parser = argument_parser{};
   parser.add_argument( flagA, "-a" ).nargs( 1 );

   auto res = parser.parse_args( { "-a", "wrong" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "-a", res.errors.front().option );
   EXPECT_EQ( argument_parser::CONVERSION_ERROR, res.errors.front().errorCode );
}

// TODO: This interface should be replaced by actions acting on ConvertedValue<CustomType>.
//   - add_argument() receives CustomType,
//   - action() receives a callable like doSetValue().
TEST( ArgumentParserConvertTest, shouldSupportCustomOptionTypes )
{
   struct CustomType
   {
      std::string value;
      std::string reversed;
   };

   class CustomValue : public argument_parser::Value
   {
      CustomType& mValue;

   public:
      CustomValue( CustomType& value )
         : mValue( value )
      {}

   protected:
      argument_parser::AssignAction getDefaultAction() override
      {
         return [this]( Value& target, const std::string& value ) {
            auto pCustom = dynamic_cast<CustomValue*>( &target );
            if ( pCustom ) {
               pCustom->mValue.value = value;
               pCustom->mValue.reversed = value;
               std::reverse( pCustom->mValue.reversed.begin(), pCustom->mValue.reversed.end() );
            }
         };
      }
   };

   CustomType custom;

   auto parser = argument_parser{};
   parser.add_argument( CustomValue( custom ), "-c" ).nargs( 1 );

   auto res = parser.parse_args( { "-c", "value" } );
   EXPECT_EQ( "value", custom.value );
   EXPECT_EQ( "eulav", custom.reversed );
}

namespace {
struct CustomType_fromstring_test
{
   std::string value;
   std::string reversed;
};
}   // namespace

namespace argparse {
template<>
struct from_string<CustomType_fromstring_test>
{
   static CustomType_fromstring_test convert( const std::string& s )
   {
      CustomType_fromstring_test custom;
      custom.value = s;
      custom.reversed = s;
      std::reverse( custom.reversed.begin(), custom.reversed.end() );
      return custom;
   }
};
}   // namespace argparse

TEST( ArgumentParserConvertTest, shouldSupportCustomOptionTypesWith_from_string )
{
   CustomType_fromstring_test custom;

   auto parser = argument_parser{};
   parser.add_argument( custom, "-c" ).nargs( 1 );

   auto res = parser.parse_args( { "-c", "value" } );
   EXPECT_EQ( "value", custom.value );
   EXPECT_EQ( "eulav", custom.reversed );
}

TEST( ArgumentParserConvertTest, shouldSupportOptionalCustomOptionTypesWith_from_string )
{
   std::optional<CustomType_fromstring_test> custom;
   std::optional<CustomType_fromstring_test> ignored;

   auto parser = argument_parser{};
   parser.add_argument( custom, "-c" ).nargs( 1 );
   parser.add_argument( ignored, "-d" ).maxargs( 1 );

   auto res = parser.parse_args( { "-c", "value" } );
   ASSERT_TRUE( static_cast<bool>( custom ) );
   EXPECT_EQ( "value", custom.value().value );
   EXPECT_EQ( "eulav", custom.value().reversed );

   EXPECT_FALSE( static_cast<bool>( ignored ) );
}

TEST( ArgumentParserConvertTest, shouldSupportVectorOfCustomTypesWith_from_string )
{
   std::vector<CustomType_fromstring_test> custom;

   auto parser = argument_parser{};
   parser.add_argument( custom, "-c" ).minargs( 1 );

   auto res = parser.parse_args( { "-c", "value", "sator" } );
   ASSERT_EQ( 2, custom.size() );
   EXPECT_EQ( "value", custom[0].value );
   EXPECT_EQ( "eulav", custom[0].reversed );
   EXPECT_EQ( "sator", custom[1].value );
   EXPECT_EQ( "rotas", custom[1].reversed );
}

TEST( ArgumentParserConvertTest, shouldSupportVectorOptions )
{
   std::vector<std::string> strings;
   std::vector<long> longs;
   std::vector<double> floats;

   auto parser = argument_parser{};
   parser.add_argument( strings, "-s" ).nargs( 1 );
   parser.add_argument( longs, "-l" ).nargs( 1 );
   parser.add_argument( floats, "-f" ).nargs( 1 );

   auto res = parser.parse_args( { "-s", "string", "-f", "12.43", "-l", "576", "-l", "981" } );
   EXPECT_TRUE( vector_eq( { "string" }, strings ) );
   EXPECT_TRUE( vector_eq( { 12.43 }, floats ) );
   EXPECT_TRUE( vector_eq( { 576, 981 }, longs ) );
}

namespace {
enum ETypeError {
   OK,
   VALUE_CONTENT,
   MAYBEVALUE_EMPTY,
   MAYBEVALUE_CONTENT,
   VECTOR_SIZE,
   VECTOR_CONTENT,
};

template<typename TValue>
ETypeError testType( const std::string& example, const TValue result,
      std::function<bool( const TValue&, const TValue& )> equal =
            []( const TValue& a, const TValue& b ) { return a == b; } )
{
   TValue value;
   std::optional<TValue> maybeValue;
   std::vector<TValue> vectorValue;
   auto parser = argument_parser{};

   parser.add_argument( value, "--value" ).nargs( 1 );
   parser.add_argument( maybeValue, "--maybe" ).nargs( 1 );
   parser.add_argument( vectorValue, "--vector" ).nargs( 1 );

   auto res =
         parser.parse_args( { "--value=" + example, "--maybe=" + example, "--vector=" + example } );

   if ( !equal( value, result ) )
      return VALUE_CONTENT;
   if ( !maybeValue )
      return MAYBEVALUE_EMPTY;
   if ( !equal( maybeValue.value(), result ) )
      return MAYBEVALUE_CONTENT;
   if ( vectorValue.empty() )
      return VECTOR_SIZE;
   if ( !equal( vectorValue.front(), result ) )
      return VECTOR_CONTENT;

   return OK;
}
}   // namespace

TEST( ArgumentParserConvertTest, shouldSupportIntegralNumericTypes )
{
   EXPECT_EQ( OK, testType<int8_t>( "123", 123 ) );
   EXPECT_EQ( OK, testType<int8_t>( "-123", -123 ) );
   EXPECT_EQ( OK, testType<uint8_t>( "234", 234 ) );

   EXPECT_EQ( OK, testType<int16_t>( "123", 123 ) );
   EXPECT_EQ( OK, testType<int16_t>( "-123", -123 ) );
   EXPECT_EQ( OK, testType<uint16_t>( "234", 234 ) );

   EXPECT_EQ( OK, testType<short>( "123", 123 ) );
   EXPECT_EQ( OK, testType<short>( "-123", -123 ) );
   EXPECT_EQ( OK, testType<unsigned short>( "234", 234 ) );

   EXPECT_EQ( OK, testType<int32_t>( "2123", 2123 ) );
   EXPECT_EQ( OK, testType<int32_t>( "-2123", -2123 ) );
   EXPECT_EQ( OK, testType<uint32_t>( "3234", 3234 ) );

   EXPECT_EQ( OK, testType<int>( "2123", 2123 ) );
   EXPECT_EQ( OK, testType<int>( "-2123", -2123 ) );
   EXPECT_EQ( OK, testType<unsigned int>( "3234", 3234 ) );

   EXPECT_EQ( OK, testType<long>( "32123", 32123 ) );
   EXPECT_EQ( OK, testType<long>( "-32123", -32123 ) );
   EXPECT_EQ( OK, testType<unsigned long>( "43234", 43234 ) );

   EXPECT_EQ( OK, testType<int64_t>( "432123", 432123 ) );
   EXPECT_EQ( OK, testType<int64_t>( "-432123", -432123 ) );
   EXPECT_EQ( OK, testType<uint64_t>( "543234", 543234 ) );

   EXPECT_EQ( OK, testType<long long>( "432123", 432123 ) );
   EXPECT_EQ( OK, testType<long long>( "-432123", -432123 ) );
   EXPECT_EQ( OK, testType<unsigned long long>( "543234", 543234 ) );
}

TEST( ArgumentParserConvertTest, shouldSupportFloatingNumericTypes )
{
   auto near = []( const auto& a, const auto& b ) { return abs( a - b ) < 1e-4; };
   EXPECT_EQ( OK, testType<float>( "123.45", 123.45, near ) );
   EXPECT_EQ( OK, testType<float>( "-123.45", -123.45, near ) );

   EXPECT_EQ( OK, testType<double>( "2123.45", 2123.45, near ) );
   EXPECT_EQ( OK, testType<double>( "-2123.45", -2123.45, near ) );

   EXPECT_EQ( OK, testType<long double>( "32123.45", 32123.45, near ) );
   EXPECT_EQ( OK, testType<long double>( "-32123.45", -32123.45, near ) );
}

TEST( ArgumentParserConvertTest, shouldSupportBoolType )
{
   EXPECT_EQ( OK, testType<bool>( "1", true ) );
   EXPECT_EQ( OK, testType<bool>( "111", true ) );
   EXPECT_EQ( OK, testType<bool>( "-111", true ) );
   EXPECT_EQ( OK, testType<bool>( "0", false ) );
}

namespace {

class StringConstructible
{
public:
   std::string value;

public:
   StringConstructible( const StringConstructible& ) = default;
   StringConstructible( const std::string& v = "" )
      : value( v )
   {}
};

class StringAssignable
{
public:
   std::string value;
   StringAssignable& operator=( const StringAssignable& ) = default;
   StringAssignable& operator=( const std::string& v )
   {
      value = v;
      return *this;
   }
};

}   // namespace

TEST( ArgumentParserConvertTest, shouldConvertValuesWithStringAssignmentOp )
{
   StringAssignable assign;

   auto parser = argument_parser{};
   parser.add_argument( assign, "--assign" ).nargs( 1 );
   auto res = parser.parse_args( { "--assign", "Assign" } );

   EXPECT_EQ( "Assign", assign.value );
}

TEST( ArgumentParserConvertTest, shouldConvertValuesWithStringConstructor )
{
   StringConstructible construct;

   auto parser = argument_parser{};
   parser.add_argument( construct, "--construct" ).nargs( 1 );
   auto res = parser.parse_args( { "--construct", "Construct" } );

   EXPECT_EQ( "Construct", construct.value );
}
