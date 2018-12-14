#include <gtest/gtest.h>
#include "../src/argparser.h"

TEST( CArgumentParserTest, shouldParseShortArguments )
{
   std::string value;

   CArgumentParser parser;
   parser.addParameter( value ).shortName( "v" );
   parser.parseArguments( { "-v", "success" } );

   EXPECT_EQ( "success", value );
}

TEST( CArgumentParserTest, shouldParseLongArguments )
{
   std::string value;

   CArgumentParser parser;
   parser.addParameter( value ).shortName( "v" ).longName( "value" );
   parser.parseArguments( { "--value", "success" } );

   EXPECT_EQ( "success", value );
}

TEST( CArgumentParserTest, shouldParseIntegerValues )
{
   long value;

   CArgumentParser parser;
   parser.addParameter( value ).shortName( "v" ).longName( "value" );
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value );
}
