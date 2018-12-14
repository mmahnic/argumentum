#include <gtest/gtest.h>
#include "../src/argparser.h"

TEST( CArgumentParserTest, shouldParseShortArguments )
{
   std::optional<std::string> value;

   CArgumentParser parser;
   parser.addParameter( value ).shortName( "v" );
   parser.parseArguments( { "-v", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( CArgumentParserTest, shouldParseLongArguments )
{
   std::optional<std::string> value;

   CArgumentParser parser;
   parser.addParameter( value ).shortName( "v" ).longName( "value" );
   parser.parseArguments( { "--value", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( CArgumentParserTest, shouldParseIntegerValues )
{
   std::optional<long> value;

   CArgumentParser parser;
   parser.addParameter( value ).shortName( "v" ).longName( "value" );
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
}

TEST( CArgumentParserTest, shouldNotSetOptionalValuesWithoutArguments )
{
   std::optional<long> value;
   std::optional<std::string> unused;

   CArgumentParser parser;
   parser.addParameter( value ).shortName( "v" ).longName( "value" );
   parser.addParameter( unused ).longName( "unused" );
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
   EXPECT_FALSE( bool(unused) );
}
