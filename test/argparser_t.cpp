#include <gtest/gtest.h>
#include "../src/argparser.h"

TEST( CArgumentParserTest, shouldParseShortOptions )
{
   std::optional<std::string> value;

   CArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).hasArgument();
   parser.parseArguments( { "-v", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( CArgumentParserTest, shouldParseLongOptions )
{
   std::optional<std::string> value;

   CArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.parseArguments( { "--value", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( CArgumentParserTest, shouldParseIntegerValues )
{
   std::optional<long> value;

   CArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
}

TEST( CArgumentParserTest, shouldNotSetOptionValuesWithoutArguments )
{
   std::optional<long> value;
   std::optional<std::string> unused;

   CArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.addOption( unused ).longName( "unused" );
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
   EXPECT_FALSE( bool(unused) );
}

TEST( CArgumentParserTest, shouldOnlyAddOptionValueIfRequired )
{
   std::optional<long> value;
   std::optional<std::string> flag;

   CArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.addOption( flag ).longName( "flag" );

   parser.parseArguments( { "--value", "2314", "--flag", "notused" } );

   EXPECT_EQ( 2314, value.value() );

   // The parameters that do not require an argumet will be given the value "1".
   // TODO: This can be changed with the flagValue() parameter option.
   EXPECT_EQ( "1", flag.value() );
}
