#include <gtest/gtest.h>
#include "../src/argparser.h"

TEST( CArgumentParserTest, shouldParseSingleArgument )
{
   std::string value;

   CArgumentParser parser;
   parser.addParameter( value ).shortName( "v" );
   parser.parseArguments( { "-v", "success" } );

   EXPECT_EQ( "success", value );
}
