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
